
#include <stdio.h>

#include "tusb.h"
#include "tusb_option.h"

#include "bsp/board.h"

static bool serialReady = false;
static bool bootBtn = false;


void cdc_task() {
    if (tud_cdc_connected()) {
        char ch = 0;

        if(tud_cdc_available()) {
            ch = tud_cdc_read_char();
        } else {
            return;
        }
    }
}

bool cdc_write(const char* str) {
    int loopCount = 0;
    while (true) {
        // Because could be in a loop waiting, don't know if this is needed
        if(serialReady) {
            tud_task();
            cdc_task();
        }

        if(loopCount == 100) break; //Need to break out at somepoint, arbitary number
        ++loopCount;

        if(!tud_hid_ready()) continue; //Loop round until ready

        if (serialReady) cdc_task();

        board_led_write(serialReady);

        if (serialReady) {
            tud_cdc_write_str(str);
            tud_cdc_write_flush();

            return true;
        } else {
            return false;
        }

    }
    return false; //Should not get to here
}


//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
	// TODO not Implemented
	(void)itf;
	(void)report_id;
	(void)report_type;
	(void)buffer;
	(void)reqlen;

	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
	// This example doesn't use multiple report and report ID
	(void)itf;
	(void)report_id;
	(void)report_type;

}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
	(void)itf;
	(void)rts;
	
	serialReady = dtr;
}

void serial_print(const char* str) {
	if (serialReady) {
		tud_cdc_write_str(str);
		tud_cdc_write_flush();
	}
}