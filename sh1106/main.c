// oled test program
// Written by Larry Bank

#include "ss_oled.h"
#include <stdio.h>

// data structure for OLED object
SSOLED ssoled;
unsigned char ucBackBuf[1024];

int main()
{
    int ret = oled_init(&ssoled, 1,
                     OLED_SH1106_3C, OLED_128x64, 0x3c,
                     0, 0);

    if (ret == OLED_NOT_FOUND)
    {
        printf("Unable to initialize I2C bus 0-2,"
               " please check your connections and verify"
               " the device address by typing 'i2cdetect -y <channel>\n");
        return 1;
    }
    
    char *msgs[] = {(char *) "SSD1306 @ 0x3C",
                    (char *) "SSD1306 @ 0x3D",
                    (char *) "SH1106 @ 0x3C",
                    (char *) "SH1106 @ 0x3D"};
                    
    oled_set_backbuffer(&ssoled, ucBackBuf);
    
    // fill with black
    oled_fill(&ssoled, 0, 1);
    oled_string_write(&ssoled, 0, 0, 0, msgs[ret], FONT_NORMAL, 0, 1);
    oled_string_write(&ssoled, 0, 0, 1, "SS_OLED Library!", FONT_NORMAL, 0, 1);
    oled_string_write(&ssoled, 0, 3, 2, "BIG!", FONT_LARGE, 0, 1);
    oled_string_write(&ssoled, 0, 0, 5, "Small", FONT_SMALL, 0, 1);
    
    for (int i = 0; i < 64; ++i)
    {
        oled_set_pixel(&ssoled, i, 16+i, 1, 1);
        oled_set_pixel(&ssoled, 127-i, 16+i, 1, 1);
    }
    
    printf("Press ENTER to quit\n");
    getchar();
    oled_power(&ssoled, 0);

    return 0;
}

