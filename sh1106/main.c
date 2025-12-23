// oled test program
// Written by Larry Bank
//
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ss_oled.h"

// data structure for OLED object
SSOLED ssoled;
unsigned char ucBackBuf[1024];

int main()
{
    // typical address; it can also be 0x3d
    int iOLEDAddr = 0x3c;
    int iChannel = 1;
    
    // Change this for your specific display
    int iOLEDType = OLED_128x64;
    int bFlip = 0;
    int bInvert = 0;
    
    int i = oledInit(&ssoled, iOLEDType, iOLEDAddr, bFlip, bInvert, iChannel, -1);

    if (i == OLED_NOT_FOUND)
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
                    
    printf("Successfully opened I2C bus %d\n", iChannel);
    oledSetBackBuffer(&ssoled, ucBackBuf);
    
    // fill with black
    oledFill(&ssoled, 0, 1);
    oledWriteString(&ssoled, 0, 0, 0, msgs[i], FONT_NORMAL, 0, 1);
    oledWriteString(&ssoled, 0, 0, 1, "SS_OLED Library!", FONT_NORMAL, 0, 1);
    oledWriteString(&ssoled, 0, 3, 2, "BIG!", FONT_LARGE, 0, 1);
    oledWriteString(&ssoled, 0, 0, 5, "Small", FONT_SMALL, 0, 1);
    
    for (i = 0; i < 64; ++i)
    {
        oledSetPixel(&ssoled, i, 16+i, 1, 1);
        oledSetPixel(&ssoled, 127-i, 16+i, 1, 1);
    }
    
    printf("Press ENTER to quit\n");
    getchar();
    oledPower(&ssoled, 0);

    return 0;
}

