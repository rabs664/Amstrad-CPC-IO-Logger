#include <stdio.h>
#include "bsp/board.h"
#include "tusb.h"
#include "simpleCDC.h"

#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "iowr.pio.h"
#include "csrd.pio.h"

#include "GPIOData.h"
#include "GPIOPins.h"

#include "bus_trans.h"
#include "IOAddr.h"

#include "GAInfo.h"


/* Debug Flags */

bool debugCRTCIndex = false;            //When a CRTC Index Write is detected
bool debugCRTCData = false;             //When a CRTC DATA Write is detected
bool debugRRead = false;                //When the read index in the the register buffer is reset
bool debugPrintRegBuffer = true;       //When instructed print the register buffer
bool debugGAPen = false;                //When a GA Pen is selected
bool debugGACol = false;                //When GA col is selected
bool debugGARMR = false;                //When RMR is selected
bool debugGAMMR = false;                //When MMR is selected
bool debugGARMR2 = false;               //When RMR2 is selected
bool debugIOLog = false;                //When an IO Log message is printed
bool debugCRTCDataTimingV1 = false;     //When a CRTC Data Write is detected print the timing information
bool debugCRTCDataTimingV2 = true;     //When a CRTC Data Write is detected print the timing information

//0..38 reset the read index into the register buffer
#define PRINT_REG_BUFFER            50  //Code to print out the register buffer
#define TOGGLE_PRINT_CRTC_INDEX     51  //Code to print CRTC Index Writes
#define TOGGLE_PRINT_CRTC_DATA      52  //Code to print CRTC Data Writes
#define TOGGLE_GA_PEN               53  //Code to print GA Pen Writes
#define TOGGLE_GA_COL               54  //Code to print GA Colour Writes
#define TOGGLE_GA_RMR               55  //Code to print GA RMR Writes
#define TOGGLE_GA_MMR               56  //Code to print GA MMR Writes
#define TOGGLE_GA_RMR2              57  //Code to print GA RMR2 Writes
#define TOGGLE_IOLOG                58  //Code to print Error Messages
#define TOGGLE_CRTC_DATA_TIMING_V1  59  //Code to print Timing Messages
#define TOGGLE_CRTC_DATA_TIMING_V2  60  //Code to print Timing Messages




/* Register Index Definitions

REGISTER BUFFER V2 (RMR, MMR and RMR2 are not decoded)																													
													 																														
0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31	32	33	34	35		36		37		38
CRTC Regsieter Values												GA Pen Colour Values																										
R0	R1	R2	R3	R4	R5	R6	R7	R8	R9	R10	R11	R12	R13	R14	R15	LR	P0	P1	P2	P3	P4	P5	P6	P7	P8	P9	P10	P11	P12	P13	P14	P15	P16	LP	GA RMR	GA MMR	GA RMR2	ERR

*/

// CRTC Registers
#define FIRST_CRTC_REGISTER_INDEX 0
#define LAST_CRTC_REGISTER_INDEX 15
#define LAST_SELECTED_CRTC_REGISTER_INDEX 16 //LR
#define MAX_CRTC_REGISTER 15

// GA Pen
#define FIRST_GA_PEN_INDEX 17
#define LAST_GA_PEN_INDEX  33
#define LAST_SELECTED_GA_PEN_INDEX 34       //LP
#define MAX_GA_PEN 16

// GA RMR
#define GA_RMR_INDEX 35

// GA MMR
#define GA_MMR_INDEX 36 

// GA RMR2
#define GA_RMR2_INDEX 37

// Error
#define ERROR_INDEX 38

// Last
#define LAST_REGISTER_INDEX 38


// Error Codes
#define INVALID_CRTC_REGISTER 1             //When the selected CRTC Register > 15
#define INVALID_GA_PEN 2                    //When the selected Pen is > 16
#define SERIAL_NOT_READY 3                  //When the serial connection is not ready to send data
#define TIME_ERROR 4                       //When the timer value goes backwards which shouldn't happen, this is just a safeguard against underflowing the time between writes and reporting a huge time between writes
#define BUFFER_OVERRUN 5                   //When a Capture Buffer Overrun has been detected


/* Buffer definitions */

#define CAPTURE_BUFFER_SIZE 8192
#define REGISTER_BUFFER_SIZE 64

#define CAPTURE_BUFFER_RING_BITS 15 // 32,768 byte buffer (2^15) entries
#define REGISTER_BUFFER_RING_BITS 8 // 256 byte buffer (2^8) entries

uint32_t captureBuffer[CAPTURE_BUFFER_SIZE] __attribute__((aligned(32768)));
uint32_t registerBuffer[REGISTER_BUFFER_SIZE] __attribute__((aligned(256)));
uint32_t timingBuffer[CAPTURE_BUFFER_SIZE] __attribute__((aligned(32768))); 

/* Global Variables */

uint32_t A0_A1;
uint32_t A8_A15;
uint32_t D0_D7;

uint32_t writeIndex = 0;
uint32_t readIndex = 0;

uint32_t selectedCRTCRegister = 0;
uint32_t selectedGAPen = 0;

uint32_t lastCRTCDataWrite = 0;
uint32_t timeSinceLastWrite = 0;

//Lag status
int lagInMin = 0, lagInMax = 8192; 
int lagOutMin = 1, lagOutMax = 10; 


void iowr(PIO pio, uint sm, uint offset) {
    iowr_program_init(pio, sm, offset);
    pio_sm_set_enabled(pio, sm, true);

    pio->txf[sm] = 125000000;
}

void csrd(PIO pio, uint sm, uint offset) {
    csrd_program_init(pio, sm, offset);
    pio_sm_set_enabled(pio, sm, true);

    pio->txf[sm] = 125000000;
}


int main()
{

    setup_bus_trans();

    PIO pioCap = pio0;
    uint offsetrx = pio_add_program(pioCap, &iowr_program);
    uint smCap = pio_claim_unused_sm(pioCap, true);
    iowr(pioCap, smCap, offsetrx);


    PIO pioReg = pio0;
    uint offsetx = pio_add_program(pioReg, &csrd_program);
    uint smReg = pio_claim_unused_sm(pioReg, true);
    csrd(pioReg, smReg, offsetx);

    setup_gpio_pins(pio0);

    board_init();
	tusb_init();
    
    /* Timing Buffer DMA*/
    int timeChan = dma_claim_unused_channel(true);
    int capChan = dma_claim_unused_channel(true);

    dma_channel_config timeCfg = dma_channel_get_default_config(timeChan);
    channel_config_set_transfer_data_size(&timeCfg, DMA_SIZE_32);
    channel_config_set_read_increment(&timeCfg, false);
    channel_config_set_write_increment(&timeCfg, true);
    channel_config_set_ring(&timeCfg, true, CAPTURE_BUFFER_RING_BITS);
    channel_config_set_chain_to(&timeCfg, capChan); // Chain the Capture Buffer DMA to the Timing Buffer DMA so that they stay in sync


    dma_channel_configure(
        timeChan,
        &timeCfg,
        timingBuffer,
        &timer_hw->timerawl,    //Everytime something is received place it in the Timing Buffer
        1,
        false
    );


    /* Capture Buffer DMA*/
    dma_channel_config capCfg = dma_channel_get_default_config(capChan);
    channel_config_set_transfer_data_size(&capCfg, DMA_SIZE_32);
    channel_config_set_read_increment(&capCfg, false);
    channel_config_set_write_increment(&capCfg, true);
    channel_config_set_dreq(&capCfg, pio_get_dreq(pioCap, smCap, false));
    channel_config_set_ring(&capCfg, true, CAPTURE_BUFFER_RING_BITS);
    channel_config_set_chain_to(&capCfg, timeChan); // Chain the Capture Buffer DMA to the Timing Buffer DMA so that they stay in sync

    dma_channel_configure(
        capChan,
        &capCfg,
        captureBuffer,
        &pioCap->rxf[0],    //Everytime something is received place it in the Capture Buffer
        1,
        true
    );
    
    /*Register Buffer Data DMA*/
    int chanReg = dma_claim_unused_channel(true);
    dma_channel_config cfgReg = dma_channel_get_default_config(chanReg);
    channel_config_set_transfer_data_size(&cfgReg, DMA_SIZE_32);
    channel_config_set_read_increment(&cfgReg, true);
    channel_config_set_write_increment(&cfgReg, false);
    channel_config_set_dreq(&cfgReg, pio_get_dreq(pioReg, smReg, true));
    channel_config_set_ring(&cfgReg, false, REGISTER_BUFFER_RING_BITS);

    dma_channel_configure(
        chanReg,
        &cfgReg,
        &pioReg->txf[smReg],   
        &registerBuffer[0],     //Take what is in the current Register Buffer Index and place it in the TX FIFO          
        1, 
        false                   //Triggered by the Register Buffer Control DMA
    );

    /*Register Buffer Control DMA*/
    int chanCtrl = dma_claim_unused_channel(true);
    dma_channel_config cfg_ctrl = dma_channel_get_default_config(chanCtrl);
    channel_config_set_transfer_data_size(&cfg_ctrl, DMA_SIZE_32);
    channel_config_set_dreq(&cfg_ctrl, pio_get_dreq(pioReg, smReg, false));
    channel_config_set_read_increment(&cfg_ctrl, false);
    channel_config_set_write_increment(&cfg_ctrl, false);
    dma_channel_configure(
        chanCtrl,
        &cfg_ctrl,
        &dma_hw->ch[chanReg].al1_transfer_count_trig,
        &pioReg->rxf[smReg],    //When something is placed on the RX FIFO use this as a trigger for the Register Buffer DMA
        0xFFFFFFFF,
        true
    );

    pio_sm_clear_fifos(pioReg, smReg); //Had to do this

    /*Main Loop*/
    while (true){
        tud_task();
        cdc_task();

        writeIndex = ((uint32_t*)dma_channel_hw_addr(capChan)->al1_write_addr - captureBuffer) & (CAPTURE_BUFFER_SIZE - 1);

        while (readIndex != writeIndex) {
                A0_A1 = get_A0_A1(captureBuffer[readIndex]);
                A8_A15 = get_A8_A15(captureBuffer[readIndex]);
                D0_D7 = get_D0_D7(captureBuffer[readIndex]);

                if (A8_A15 == CRTC_INDEX_PORT) {
                    if (debugCRTCIndex) {
                        char buf[30] = {0};
                        sprintf(buf, "CRTCI %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);

                        if(!cdc_write(buf)) {
                                  registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }
                    }

                    // Only update the selected CRTC register if the value is a valid register index, otherwise keep the previously selected register
                    if(D0_D7 <= MAX_CRTC_REGISTER) {
                        selectedCRTCRegister = D0_D7;
                        registerBuffer[LAST_SELECTED_CRTC_REGISTER_INDEX] = selectedCRTCRegister;
                    } else {
                        registerBuffer[ERROR_INDEX] = INVALID_CRTC_REGISTER;
                    }
                

                } else if (A8_A15 == CRTC_DATA_PORT) {
                    if (debugCRTCData) {
                        char buf[30] = {0};
                        sprintf(buf, "CRTCD %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);

                        if(!cdc_write(buf)) {
                                registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }
                    }

                    registerBuffer[selectedCRTCRegister] = D0_D7;

                    if(lastCRTCDataWrite != 0) {
                        if(timingBuffer[readIndex] >= lastCRTCDataWrite) {
                            timeSinceLastWrite = timingBuffer[readIndex] - lastCRTCDataWrite;
                        } else {
                            timeSinceLastWrite = 0;

                            registerBuffer[ERROR_INDEX] = TIME_ERROR; // Set error flag if the timer value has somehow gone backwards, this shouldn't happen but just in case it does we don't want to underflow and report a huge time between writes
                        }
                    }

                    if(debugCRTCDataTimingV1) {
                        int lag = (writeIndex > readIndex) ? (writeIndex - readIndex) : ((CAPTURE_BUFFER_SIZE - readIndex) + writeIndex); 
                        int lagScaled = ((lag - lagInMin) * (lagOutMax - lagOutMin) / (lagInMax - lagInMin)) + lagOutMin; 

                        char buf[40] = {0};
                        sprintf(buf, "%010u %04u %04u %02u %02x %02u\n\r", timeSinceLastWrite, readIndex, writeIndex, selectedCRTCRegister, D0_D7, lagScaled);


                        if(!cdc_write(buf)) {
                            registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }

                    } else if(debugCRTCDataTimingV2) {
                        int lag = (writeIndex > readIndex) ? (writeIndex - readIndex) : ((CAPTURE_BUFFER_SIZE - readIndex) + writeIndex); 
                        int lagScaled = ((lag - lagInMin) * (lagOutMax - lagOutMin) / (lagInMax - lagInMin)) + lagOutMin; 

                        char buf[30] = {0};
                        sprintf(buf, "%010u %02u %02x %02u\n\r", timeSinceLastWrite, selectedCRTCRegister, D0_D7, lagScaled);

                        if(!cdc_write(buf)) {
                            registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }
                    }

                    lastCRTCDataWrite = timingBuffer[readIndex];
                

                } else if (A8_A15 == CPC_IO_CAPTURE) {

                    if(D0_D7 <= LAST_REGISTER_INDEX) {
                        if(debugRRead) {
                            char buf[30] = {0};
                            sprintf(buf, "RREAD %02u\n\r", D0_D7);

                            if(!cdc_write(buf)) {
                                registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                            }
                        }

                        dma_hw->ch[chanReg].read_addr = (uint32_t)&registerBuffer[0] + (D0_D7 * 4);
                    }


                    switch(D0_D7) {
                        case PRINT_REG_BUFFER:
                            if(debugPrintRegBuffer) {
                                char hdr[6] = {0};
                                sprintf(hdr, "REGBF");

                                if(!cdc_write(hdr)) {
                                    registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                                    break;
                                }

                                char buf[3] = {0};
                                for (int i = 0; i <= LAST_REGISTER_INDEX; i++) {
                                    sprintf(buf, " %02x", registerBuffer[i] & 0xFF);

                                    if(!cdc_write(buf)) {
                                        registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                                        break;
                                    }
                                }       
                                    
                                char crl[3] = {"\n\r"};
                                if(!cdc_write(crl)) {
                                    registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                                    break;
                                }
                            }
                            break;

                        case TOGGLE_PRINT_CRTC_INDEX:
                            debugCRTCIndex = !debugCRTCIndex;
                            break;

                        case TOGGLE_PRINT_CRTC_DATA:
                            debugCRTCData = !debugCRTCData;
                            break;

                        case TOGGLE_GA_PEN:
                            debugGAPen = !debugGAPen;
                            break;

                        case TOGGLE_GA_COL:
                            debugGACol = !debugGACol;
                            break;

                        case TOGGLE_GA_RMR:
                            debugGARMR = !debugGARMR;
                            break;
                            
                        case TOGGLE_GA_MMR:
                            debugGAMMR = !debugGAMMR;
                            break;

                        case TOGGLE_GA_RMR2:
                            debugGARMR2 = !debugGARMR2;
                            break;
                            
                        case TOGGLE_IOLOG:
                            debugIOLog = !debugIOLog;
                            break;

                        case TOGGLE_CRTC_DATA_TIMING_V1:
                            debugCRTCDataTimingV1 = !debugCRTCDataTimingV1;
                            break;

                        case TOGGLE_CRTC_DATA_TIMING_V2:
                            debugCRTCDataTimingV2 = !debugCRTCDataTimingV2; 
                            break;
                        }

                    if(debugIOLog) {
                        char buf[30] = {0};
                        sprintf(buf, "IOLOG %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);

                        if(!cdc_write(buf)) {
                            registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }
                    } 
                
  
                } else if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_PEN_FUNCTION)) {
                    if(debugGAPen) {
                        char buf[30] = {0};
                        sprintf(buf,"GAPEN %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);
  
                        if(!cdc_write(buf)) {
                            registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }
                    }

                    if(D0_D7 <= MAX_GA_PEN) {
                        selectedGAPen = get_GA_PEN(D0_D7);
                        registerBuffer[LAST_SELECTED_GA_PEN_INDEX] = selectedGAPen;
                    } else {
                        registerBuffer[ERROR_INDEX] = INVALID_GA_PEN;
                    }

            
                } else if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_PEN_COLOUR_FUNCTION)) {
                    if(debugGACol) {
                        char buf[30] = {0};
                        sprintf(buf, "GACOL %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);

                        if(!cdc_write(buf)) {
                            registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }
                    }
  
                    registerBuffer[selectedGAPen + FIRST_GA_PEN_INDEX] = get_GA_COLOUR(D0_D7);
                        


                } else if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_RMR_FUNCTION)) {

                    if(!is_RMR2(D0_D7)) {
                        if(debugGARMR) {
                            char buf[30] = {0};
                            sprintf(buf, "GARMR %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);

                            if(!cdc_write(buf)) {
                                registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                            }
                        }
                        registerBuffer[GA_RMR_INDEX] = D0_D7;

                    } else {
                        if(debugGARMR2) {
                            char buf[30] = {0};
                            sprintf(buf, "GARM2 %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);

                            if(!cdc_write(buf)) {
                                registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                            }
                        }
                        registerBuffer[GA_RMR2_INDEX] = D0_D7;
                    }
                
                
                } else if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_MMR_FUNCTION)) {
                    if(debugGAMMR) {
                        char buf[30] = {0};
                        sprintf(buf, "GAMMR %04u %04u %02x %02x %02x\n\r", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);

                        if(!cdc_write(buf)) {
                            registerBuffer[ERROR_INDEX] = SERIAL_NOT_READY;
                        }
                    }
                    registerBuffer[GA_MMR_INDEX] = D0_D7;
                }

                readIndex = (readIndex + 1) & (CAPTURE_BUFFER_SIZE - 1);
        }

    }   
}