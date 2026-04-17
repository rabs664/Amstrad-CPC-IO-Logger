#include <stdio.h>

void cdc_task();

bool cdc_write(const char* str);

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts);