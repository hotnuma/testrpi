// oled test program
// Written by Larry Bank

#include "ss_oled.h"
#include <stdio.h>

// data structure for oled object
SSOLED oled;
unsigned char ucBackBuf[1024];

int main()
{
    if (oled_init(&oled, 1, 0x3c,
                  OLED_SH1106, OLED_128x64,
                  false, false) == OLED_NOT_FOUND)
        return 1;
    
    oled_set_backbuffer(&oled, ucBackBuf);
    
    // fill with black
    oled_fill(&oled, 0, 1);

    oled_string_write(&oled, 0, 0, 0, "Line 1", FONT_16x16, false, true);
    oled_string_write(&oled, 0, 0, 2, "Line 1", FONT_16x16, false, true);
    oled_string_write(&oled, 0, 0, 4, "Line 1", FONT_16x16, true, true);
    oled_string_write(&oled, 0, 0, 6, "12345678", FONT_16x16, false, true);
    //oled_string_write(&oled, 0, 0, 1, "SS_OLED Library!", FONT_NORMAL, 0, 1);
    
    printf("Press ENTER to quit\n");
    getchar();
    oled_power(&oled, 0);

    return 0;
}

