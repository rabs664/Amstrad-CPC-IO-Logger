/* GPIO Pins for CPC IO Logger */

#include "GPIOPins.h"

void setup_gpio_pins(PIO pio) {
    // PIO IN pins GPIO 0 to GPIO 17 = A0..A1 + A8..A15 + D0..D7 + DBGC
    for (int i = P_A0; i <= P_DBGC; i++) {
        pio_gpio_init(pio,i);
    }

    // GPIO 27 = IOWR, value 0 = active
    pio_gpio_init(pio,P_IOWR);

    // GPIO 22 = DATA_AB, value of 1 = CPC to PICO, 0 = PICO to CPC
    pio_gpio_init(pio,P_DATA_AB);

    // GPIO 26 = CSRD, value 0 = active
    pio_gpio_init(pio,P_CSRD);

}