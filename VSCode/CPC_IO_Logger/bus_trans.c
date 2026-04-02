#include "hardware/pio.h"
#include "bus_trans.h"
#include "GPIOPins.h"

// Bus Transceiver control pin definitions
// This file contains the definitions and functions for controlling the bus transceivers
// that are used to interface with the Amstrad CPC.
//

// Enable the three bus transceivers
void setup_bus_trans(void) {
    // Data
    gpio_init(P_DATA_CE);
    gpio_set_dir(P_DATA_CE, GPIO_OUT);
    gpio_put(P_DATA_CE, ENABLE);

    // CTRL
    gpio_init(P_CTRL_CE);
    gpio_set_dir(P_CTRL_CE, GPIO_OUT);
    gpio_put(P_CTRL_CE, ENABLE);

    // ADHB
    gpio_init(P_ADHB_CE);
    gpio_set_dir(P_ADHB_CE, GPIO_OUT);
    gpio_put(P_ADHB_CE, ENABLE);

}