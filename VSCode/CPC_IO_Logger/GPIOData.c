    /*
    * GPIOData.c
    *
    *  Created on: 17.06.2024
    *
    *  This file contains the definitions to extract the captured data from the GPIO pin raw data.
    * 
    *  The raw data is captured in the format
    *  A0_A1 | A8_A15 | D0_D7
    */
    #include "hardware/pio.h"

    #include "GPIOData.h"


    uint32_t get_A0_A1(uint32_t rawData) {
        return (rawData & A0_A1_MASK) >> A0_A1_SHIFT;
    }

    uint32_t get_A8_A15(uint32_t rawData) {
        return (rawData & A8_A15_MASK) >> A8_A15_SHIFT;
    }

    uint32_t get_D0_D7(uint32_t rawData) {
        return (rawData & D0_D7_MASK) >> D0_D7_SHIFT;
    }  