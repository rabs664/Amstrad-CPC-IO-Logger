// This file contains the definitions for the Gate Array registers of the Amstrad CPC.

// References:
// https://cpctech.cpcwiki.de/docs/garray.html
// https://www.cpcwiki.eu/index.php/Gate_Array


#include "hardware/pio.h"

#define GA_FUNCTION_SELECT_MASK 0XC0 // Bits 6 and 7 of the data are used to select the function of the Gate Array port
#define GA_FUNCTION_SELECT_SHIFT 6

/*
Bit 7	Bit 6	Function
0	0	Select pen
0	1	Select colour for selected pen
1	0	Select screen mode, rom configuration and interrupt control
1	1	Ram Memory Management (note 1)
*/

#define GA_SELECT_PEN_FUNCTION 0x00
#define GA_SELECT_PEN_COLOUR_FUNCTION 0x01
#define GA_SELECT_RMR_FUNCTION 0x02
#define GA_SELECT_MMR_FUNCTION 0x03

/*
Bit	Value	Function
7	0	Gate Array function "Pen Selection"
6	0
5	x	not used
4	1	Select border
3	x	ignored
2	x	ignored
1	x	ignored
0	x	ignored
*/

//#define GA_SELECT_BORDER_MASK 0x10 // Bits 4 of the data are used to select the border
//#define GA_SELECT_BORDER_MASK_SHIFT 4

/*
Bit	Value	Function
7	0	Gate Array function "Pen Selection"
6	0
5	x	not used
4	0	Select pen
3	x	Pen Number
2	x
1	x
0	x
*/

// But mask as bits 0-4, handling the border as pen 16, to simplify the code when the border is selected as a pen

#define GA_SELECT_PEN_MASK 0x1F // Bits 0-4 of the data are used to select the pen

/*
Bit	Value	Function
7	0	Gate Array function "Colour Selection"
6	1
5	x	not used
4	x	Colour number
3	x
2	x
1	x
0	x
*/

#define GA_SELECT_COLOUR_MASK 0x1F // Bits 0-4 of the data are used to select the colour

/*
Colour Number	Colour Name
0	White
1	White (note 1)
2	Sea Green
3	Pastel Yellow
4	Blue
5	Purple
6	Cyan
7	Pink
8	Purple (note 1)
9	Pastel Yellow (note 1)
10	Bright Yellow
11	Bright White
12	Bright Red
13	Bright Magenta
14	Orange
15	Pastel Magenta
16	Blue (note 1)
17	Sea Green (note 1)
18	Bright Green
19	Bright Cyan
20	Black
21	Bright Blue
22	Green
23	Sky Blue
24	Magenta
25	Pastel Green
26	Lime
27	Pastel Cyan
28	Red
29	Mauve
30	Yellow
31	Pastel Blue
*/



/*
Bit	Value	Function
7	1	Gate Array RMR register
6	0
5	-	must be 0 on Plus machines with ASIC unlocked
4	x	Interrupt generation control
3	x	1=Upper ROM area disable, 0=Upper ROM area enable
2	x	1=Lower ROM area disable, 0=Lower ROM area enable
1	x	Graphics Mode selection
0	x
*/
//#define GA_RMR_INTERRUPT_CONTROL_MASK       0x10 // Bit 4 of the data is used to control the interrupt generation
//#define GA_RMR_INTERRUPT_CONTROL_MASK_SHIFT 4
//#define GA_RMR_UPPER_ROM_DISABLE_MASK       0x08 // Bit 3 of the data is used to disable the upper ROM area
//#define GA_RMR_UPPER_ROM_DISABLE_MASK_SHIFT 3
//#define GA_RMR_LOWER_ROM_DISABLE_MASK       0x04 // Bit 2 of the data is used to disable the lower ROM area
//#define GA_RMR_LOWER_ROM_DISABLE_MASK_SHIFT 2

/*
Bit 1	Bit 0	Screen mode
0	0	Mode 0, 160x200 resolution, 16 colours
0	1	Mode 1, 320x200 resolution, 4 colours
1	0	Mode 2, 640x200 resolution, 2 colours
1	1	Mode 3, 160x200 resolution, 4 colours (note 1)
*/

//#define GA_RMR_MODE_MASK 0x03 // Bits 0 and 1 of the data are used to select the screen mode


/*
Bit	Value	Function
7	1	Gate Array MMR register
6	1
5	x	64K bank number (0..7); always 0 on an unexpanded CPC6128, 0-7 on Standard Memory Expansions
4	x
3	x
2	x	RAM Config (0..7)
1	x
0	x
*/
//#define GA_MMR_BANK_NUMBER_MASK         0x38 // Bit 5 of the data is used to select the 64K bank number
//#define GA_MMR_BANK_NUMBER_MASK_SHIFT   3
//#define GA_MMR_RAM_CONFIG_MASK          0x07 // Bits 0-2 of the data are used to select the RAM configuration

/*

Bit	Value	Function
7	1	Gate Array RMR2 register
6	0
5	1
4	x	RMR addressing mode
3	x
2	x	Physical ROM number (0..7)
1	x
0	x
*/
#define GA_RMR2_BIT_5_MASK          0x20
#define GA_RMR2_BIT_5_MASK_SHIFT    5
//#define GA_RMR2_ADDR_MODE_MASK      0x18
//#define GA_RMR2_ADDR_MODE_SHIFT     3
//#define GA_RMR2_ROM_NUM_MASK        0x7

uint32_t get_GA_FUNCTION_SELECT(uint32_t rawData);

uint32_t get_GA_PEN(uint32_t rawData);

uint32_t get_GA_COLOUR(uint32_t rawData);

bool is_RMR2 (uint32_t rawData);