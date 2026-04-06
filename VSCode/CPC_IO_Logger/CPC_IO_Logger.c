#include <stdio.h>
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

//#define DEBUG 1

#ifdef DEBUG
    bool debugCRTCIndex = false;         //When a CRTC Index Write is detected
    bool debugCRTCData = false;          //When a CRTC DATA Write is detected
    bool debugRRead = false;             //When the read index in the the register buffer is reset
    bool debugPrintRegBuffer = false;    //When instructed print the register buffer
    bool debugGAPen = false;            //When a GA Pen is selected
    bool debugGACol = false;             //When GA col is selected
    bool debugGARMR = false;             //When RMR is selected
    bool debugGAMMR = false;             //When MMR is selected
    bool debugGARMR2 = false;           //When RMR2 is selected
    bool debugIOLog = false;            //When an IO Log message is printed


    #define PRINT_REG_BUFFER        50 //Code to print out the register buffer
    #define TOGGLE_PRINT_CRTC_INDEX 51 //Code to print CRTC Index Writes
    #define TOGGLE_PRINT_CRTC_DATA  52 //Code to print CRTC Data Writes
    #define TOGGLE_GA_PEN           53 //Code to print GA Pen Writes
    #define TOGGLE_GA_COL           54 //Code to print GA Colour Writes
    #define TOGGLE_GA_RMR           55 //Code to print GA RMR Writes
    #define TOGGLE_GA_MMR           56 //Code to print GA MMR Writes
    #define TOGGLE_GA_RMR2          57 //Code to print GA RMR2 Writes
    #define TOGGLE_IOLOG            58 //Code to print Error Messages
#endif

/* Register Index Definitions

REGISTER BUFFER																									
													 																													
0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31	32	33	34	35	        36	    37	    38	39	        40	        41	        42      43
CRTC Regsieter Values												GA Pen Colour Values								                    GA RMR				            GA MMR		            GA RMR2	
R0	R1	R2	R3	R4	R5	R6	R7	R8	R9	R10	R11	R12	R13	R14	R15	LR	P0	P1	P2	P3	P4	P5	P6	P7	P8	P9	P10	P11	P12	P13	P14	P15	P16	LP	SCREEN MODE	LROM	UROM	INT	RAM BANK	RAM CONFIG	ADDR MODE	ROM NUM ERROR

*/

// CRTC Registers
#define FIRST_CRTC_REGISTER_INDEX 0
#define LAST_CRTC_REGISTER_INDEX 15
#define LAST_SELECTED_CRTC_REGISTER_INDEX 16 //LR
#define MAX_CRTC_REGISTER 15
#define INVALID_CRTC_REGISTER 1             //When the selected CRTC Register > 15

// GA Pen
#define FIRST_GA_PEN_INDEX 17
#define LAST_GA_PEN_INDEX  33
#define LAST_SELECTED_GA_PEN_INDEX 34       //LP
#define MAX_GA_PEN 16
#define INVALID_GA_PEN 2                    //When the selected Pen is > 16

// GA RMR
#define GA_SCREEN_MODE_INDEX 35
#define GA_LOWER_ROM_DISABLE_INDEX 36
#define GA_UPPER_ROM_DISABLE_INDEX 37
#define GA_INTERRUPT_CONTROL_INDEX 38

// GA MMR
#define GA_BANK_NUMBER_INDEX 39 
#define GA_BANK_CONFIG_INDEX 40  

// GA RMR2
#define GA_ADDR_MODE_INDEX 41
#define GA_ROM_NUM_INDEX 42

// Error
#define ERROR_INDEX 43

// Last
#define LAST_REGISTER_INDEX 43


/* Buffer definitions */

#define CAPTURE_BUFFER_SIZE 1024
#define REGISTER_BUFFER_SIZE 64

#define CAPTURE_BUFFER_RING_BITS 12 // 4096 byte buffer (2^12) entries
#define REGISTER_BUFFER_RING_BITS 8 // 256 byte buffer (2^8) entries

uint32_t captureBuffer[CAPTURE_BUFFER_SIZE] __attribute__((aligned(4096)));
uint32_t registerBuffer[REGISTER_BUFFER_SIZE] __attribute__((aligned(256)));


/* Global Variables */

uint32_t A0_A1;
uint32_t A8_A15;
uint32_t D0_D7;

uint32_t writeIndex = 0;
uint32_t readIndex = 0;

uint32_t selectedCRTCRegister = 0;
uint32_t selectedGAPen = 0;



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

    #ifdef DEBUG
        stdio_init_all();
    #endif

    /* Capture Buffer DMA*/
    int capChan = dma_claim_unused_channel(true);
    dma_channel_config capCfg = dma_channel_get_default_config(capChan);
    channel_config_set_transfer_data_size(&capCfg, DMA_SIZE_32);
    channel_config_set_read_increment(&capCfg, false);
    channel_config_set_write_increment(&capCfg, true);
    channel_config_set_dreq(&capCfg, pio_get_dreq(pioCap, smCap, false));
    channel_config_set_ring(&capCfg, true, CAPTURE_BUFFER_RING_BITS);

    dma_channel_configure(
        capChan,
        &capCfg,
        captureBuffer,
        &pioCap->rxf[0],    //Everytime something is received place it in the Capture Buffer
        0xFFFFFFFF,
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

        writeIndex = ((uint32_t*)dma_channel_hw_addr(capChan)->al1_write_addr - captureBuffer) & (CAPTURE_BUFFER_SIZE - 1);

        while (readIndex != writeIndex) {

                A0_A1 = get_A0_A1(captureBuffer[readIndex]);
                A8_A15 = get_A8_A15(captureBuffer[readIndex]);
                D0_D7 = get_D0_D7(captureBuffer[readIndex]);

                if (A8_A15 == CRTC_INDEX_PORT) {
                    #ifdef DEBUG
                        if (debugCRTCIndex) {
                        printf("CRTCI %04u %04u %02x %02x %02x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);
                        }
                    #endif

                    // Only update the selected CRTC register if the value is a valid register index, otherwise keep the previously selected register
                    if(D0_D7 <= MAX_CRTC_REGISTER) {
                        selectedCRTCRegister = D0_D7;
                        registerBuffer[LAST_SELECTED_CRTC_REGISTER_INDEX] = selectedCRTCRegister;
                    } else {
                        registerBuffer[ERROR_INDEX] = INVALID_CRTC_REGISTER; // Set error flag if invalid register index is selected
                    }
        
                }

                if (A8_A15 == CRTC_DATA_PORT) {
                    #ifdef DEBUG
                        if (debugCRTCData) {
                            printf("CRTCD %04u %04u %02x %02x %02x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);
                        }
                    #endif

                    registerBuffer[selectedCRTCRegister] = D0_D7;
                }

                if (A8_A15 == CPC_IO_CAPTURE) {

                    if(D0_D7 <= LAST_REGISTER_INDEX) {
                        #ifdef DEBUG
                            if(debugRRead) {
                                printf("RREAD %02u\n", D0_D7);
                            }
                        #endif

                        dma_hw->ch[chanReg].read_addr = (uint32_t)&registerBuffer[0] + (D0_D7 * 4);
                    }

                    #ifdef DEBUG
                        switch(D0_D7) {
                            case PRINT_REG_BUFFER:
                                if(debugPrintRegBuffer) {
                                    printf("REGBF ");

                                    for (int i = 0; i <= LAST_REGISTER_INDEX; i++) {
                                        printf("%02x ", registerBuffer[i] & 0xFF);
                                    }       
                                    printf("\n");
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
                            
                        }

                        if(debugIOLog) {
                            printf("IOLOG %04u %04u %02x %02x %02x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7);
                        } 

                    #endif



                }
  

                if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_PEN_FUNCTION)) {

                    #ifdef DEBUG
                        if(debugGAPen) {
                           printf("GAPEN %04u %04u %02x %02x %02x %02x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7, get_GA_PEN(D0_D7));
                        }
                    #endif

                    if(D0_D7 <= MAX_GA_PEN) {
                        selectedGAPen = get_GA_PEN(D0_D7);
                        registerBuffer[LAST_SELECTED_GA_PEN_INDEX] = selectedGAPen;
                    } else {
                        registerBuffer[ERROR_INDEX] = INVALID_GA_PEN; // Set error flag if invalid GA Pen is selected
                    }

                }

                if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_PEN_COLOUR_FUNCTION)) {

                    #ifdef DEBUG
                        if(debugGACol) {
                            printf("GACOL %04u %04u %02x %02x %02x %02x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7, get_GA_COLOUR(D0_D7));
                        }
                    #endif

                    registerBuffer[selectedGAPen + FIRST_GA_PEN_INDEX] = get_GA_COLOUR(D0_D7);
                        
                }

                if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_RMR_FUNCTION)) {

                    if(!is_RMR2(D0_D7)) {
                        #ifdef DEBUG
                            if(debugGARMR) {
                                printf("GARMR %04u %04u %02x %02x %02x %02x %01x %01x %01x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7, get_GA_MODE(D0_D7),get_GA_LOWER_ROM_DISABLE(D0_D7),get_GA_UPPER_ROM_DISABLE(D0_D7),get_GA_INTERRUPT_CONTROL(D0_D7));
                            }      
                        #endif

                        registerBuffer[GA_SCREEN_MODE_INDEX] = get_GA_MODE(D0_D7);
                        registerBuffer[GA_LOWER_ROM_DISABLE_INDEX] = get_GA_LOWER_ROM_DISABLE(D0_D7);
                        registerBuffer[GA_UPPER_ROM_DISABLE_INDEX] = get_GA_UPPER_ROM_DISABLE(D0_D7);
                        registerBuffer[GA_INTERRUPT_CONTROL_INDEX] = get_GA_INTERRUPT_CONTROL(D0_D7);

                    } else {
                        #ifdef DEBUG
                            if(debugGARMR2) {
                                printf("GARM2 %04u %04u %02x %02x %02x %02x %02x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7, get_GA_RMR2_ADDR_MODE(D0_D7),get_GA_RMR2_ROM_NUM(D0_D7));
                            }      
                        #endif

                        registerBuffer[GA_ADDR_MODE_INDEX] = get_GA_RMR2_ADDR_MODE(D0_D7);
                        registerBuffer[GA_ROM_NUM_INDEX] = get_GA_RMR2_ROM_NUM(D0_D7);
                    
                    }
                } 
                
                if (A8_A15 == GATE_ARRAY_PORT && (get_GA_FUNCTION_SELECT(D0_D7) == GA_SELECT_MMR_FUNCTION)) {
                    #ifdef DEBUG
                        if(debugGAMMR) {
                            printf("GAMMR %04u %04u %02x %02x %02x %02x %02x\n", readIndex, writeIndex, A0_A1, A8_A15, D0_D7, get_GA_BANK_NUMBER(D0_D7), get_GA_RAM_CONFIG(D0_D7));
                        }
                    #endif

                    registerBuffer[GA_BANK_CONFIG_INDEX] = get_GA_RAM_CONFIG(D0_D7);
                    registerBuffer[GA_BANK_NUMBER_INDEX] = get_GA_BANK_NUMBER(D0_D7);

                }

                readIndex = (readIndex + 1) & (CAPTURE_BUFFER_SIZE - 1);
        }

    }   
}
