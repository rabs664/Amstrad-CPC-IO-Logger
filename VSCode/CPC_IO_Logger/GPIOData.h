    /*
    * GPIOData.h
    *
    *  Created on: 17.06.2024
    *
    *  This file contains the definitions to extract the captured data from the GPIO pin raw data.
    * 
    *  The raw data is captured in the format
    *  A0_A1 | A8_A15 | D0_D7
    */
    #include "hardware/pio.h"

    #define A0_A1_MASK  0x003
    #define A8_A15_MASK 0x3FC
    #define D0_D7_MASK  0x3FC00

    #define A0_A1_SHIFT  0
    #define A8_A15_SHIFT 2
    #define D0_D7_SHIFT  10

    uint32_t get_A0_A1(uint32_t rawData);

    uint32_t get_A8_A15(uint32_t rawData);

    uint32_t get_D0_D7(uint32_t rawData);