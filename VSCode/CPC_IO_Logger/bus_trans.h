// Bus Transceiver functions and definitions
//
// This file contains the definitions and functions for controlling the bus transceivers
// that are used to interface with the Amstrad CPC. 
//
// The bus transceivers are used to control the direction and enable/disable the data bus, 
// as well as to control the address high byte and control signals. 
//
// The functions in this file will be used to set up the bus transceivers and to control the 
// data bus direction and enable/disable the data bus as needed.    
//
// There are three bus transceivers used in this project:
// 1. Control (CTRL) - This transceiver is used to control the enable/disable of the data bus and to control the direction of the data bus.
// 2. Address High Byte (ADHB) - This transceiver is used to control the address high byte signals.
// 3. Data (DATA) - This transceiver is used to control the data bus signals. 
//
// All three transceivers are controlled using GPIO pins on the Raspberry Pi Pico, 
// and the functions in this file will use the GPIO library to control these pins.
//
// The GPIO pins used for the transceivers are defined as follows:
// CTRL_CE_GPIO - This GPIO pin is used to control the enable/disable of the control transceiver.
// ADHB_CE_GPIO - This GPIO pin is used to control the enable/disable of the address high byte transceiver.
// DATA_CE_GPIO - This GPIO pin is used to control the enable/disable of the data transceiver.
// DATA_DIR_GPIO - This GPIO pin is used to control the direction of the data bus.
//
// The Data Bus direction can be set to either CPC_TO_PICO or PICO_TO_CPC, depending on whether 
// the data is being read from the CPC or written to the CPC.
//
// Below is the layout of the three transceivers and their connections to the GPIO pins:
// 
//         +-----------------------------+
//         |        Control (CTRL)       |
//         |                             |
//         |        CE GPIO: 0           |
//         +-----------------------------+
//         |   Address High Byte (ADHB)  |
//         |                             |
//         |        CE GPIO: 1           |
//         +-----------------------------+
//         |          Data (DATA)        |
//         |                             |
//         |        CE GPIO: 2           |
//         |      Direction GPIO: 3      |
//         +-----------------------------+
//
// Below is layout of the signals conencted to the bus transceivers:
//
//         +-----------------------------+
//         |        Control (CTRL)       |
//         |                             |
//         |        CSRD GPIO: 22        |
//         |        IOWR GPIO: 26        |
//         |        A0   GPIO: 0         |
//         |        A1   GPIO: 1         |
//         +-----------------------------+
//
//         +-----------------------------+
//         |   Address High Byte (ADHB)  |
//         |                             |
//         |        A8   GPIO: 2         |
//         |        A9   GPIO: 3         |
//         |        A10  GPIO: 4         | 
//         |        A11  GPIO: 5         |
//         |        A12  GPIO: 6         |
//         |        A13  GPIO: 7         |
//         |        A14  GPIO: 8         |
//         |        A15  GPIO: 9         |
//         +-----------------------------+
//
//         +-----------------------------+
//         |          Data (DATA)        |
//         |                             |
//         |        D0   GPIO: 10        |
//         |        D1   GPIO: 11        |
//         |        D2   GPIO: 12        |
//         |        D3   GPIO: 13        |
//         |        D4   GPIO: 14        |
//         |        D5   GPIO: 15        |
//         |        D6   GPIO: 16        |
//         |        D7   GPIO: 27        |
//         +-----------------------------+
//


#define ENABLE 0
#define DISABLE 1

// Bus Transceier Direction Control
#define DATA_DIR_GPIO 21

#define CPC_TO_PICO 1
#define PICO_TO_CPC 0

// Bus Transceier setup
// Enable the three bus transceivers
void setup_bus_trans(void);