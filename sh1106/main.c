// oled test program
// Written by Larry Bank

#include "ss_oled.h"
#include <msleep.h>
#include <stdio.h>
#include <stdlib.h>

// data structure for oled object
SSOLED oled;
unsigned char buffer[1024];

int main()
{
    if (!oled_init(&oled, 1, 0x3c,
                   OLED_SH1106, OLED_128x64,
                   false, false))
        return EXIT_FAILURE;
    
    oled_set_backbuffer(&oled, buffer);

    // fill with black
    oled_fill(&oled, 0, 1);

    oled_string_write(&oled, 0, 0, 0, "Line 1", FONT_16x16, false, true);
    //msleep(1000);

    oled_string_write(&oled, 0, 0, 2, "Line 1", FONT_16x16, false, true);
    oled_string_write(&oled, 0, 0, 4, "Line 1", FONT_16x16, true, true);
    oled_string_write(&oled, 0, 0, 6, "12345678", FONT_16x16, false, true);
    
    printf("Press ENTER to quit\n");
    getchar();
    oled_power(&oled, 0);

    return EXIT_SUCCESS;
}

