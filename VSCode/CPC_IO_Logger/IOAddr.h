/*
    * IOAddr.h
    *
    *  Created on: 17.06.2024
    *
    *  This file contains the definitions for the I/O address space of the Amstrad CPC.
    * 
    *  See https://www.cpcwiki.eu/index.php/Default_I/O_Port_Summary
    * 
    * 
    I/O	    Decoded as	        Port	                                                Read	Write
    #7FXX	%01xxxxxx xxxxxxxx	Gate Array	                                                    -	Write
    #7FXX	%0xxxxxxx xxxxxxxx	PAL extension to Gate Array for 128K RAM banking                -	Write
    #BCXX	%x0xxxx00 xxxxxxxx	6845 CRTC Index	                                                -	Write
    #BDXX	%x0xxxx01 xxxxxxxx	6845 CRTC Data Out	                                            -	Write
    #BEXX	%x0xxxx10 xxxxxxxx	6845 CRTC Status (as far as supported)	                Read    -
    #BFXX	%x0xxxx11 xxxxxxxx	6845 CRTC Data In (as far as supported)	                Read	-
    #DFXX	%xx0xxxxx xxxxxxxx	Upper ROM Bank Number	                                        -	Write
    #EFXX	%xxx0xxxx xxxxxxxx	Printer Port	                                                -	Write
    #F4XX	%xxxx0x00 xxxxxxxx	8255 PPI Port A (PSG Data)	                            Read    -	Write
    #F5XX	%xxxx0x01 xxxxxxxx	8255 PPI Port B (Vsync,PrnBusy,Tape,etc.)	            Read	-
    #F6XX	%xxxx0x10 xxxxxxxx	8255 PPI Port C (KeybRow,Tape,PSG Control)	                    -	Write
    #F7XX	%xxxx0x11 xxxxxxxx	8255 PPI Control-Register	                                    -	Write

    Also defines the CPC_IO_CAPTURE address which is not an actual I/O port on the CPC but is used to trigger 
    the return of capactured CRTC and GA registers.

    F8XX 	Not decoded, but used by some software to trigger I/O capture on the Pico	-	Write
*/

#define GATE_ARRAY_PORT     0x7F
#define CRTC_INDEX_PORT     0xBC
#define CRTC_DATA_PORT      0xBD
#define CRTC_STATUS_PORT    0xBE
#define CRTC_DATA_IN_PORT   0xBF
#define UPPER_ROM_BANK_PORT 0xDF
#define PRINTER_PORT        0xEF
#define PPI_PORT_A          0xF4
#define PPI_PORT_B          0xF5
#define PPI_PORT_C          0xF6
#define PPI_CONTROL_PORT    0xF7

#define CPC_IO_CAPTURE      0xF9