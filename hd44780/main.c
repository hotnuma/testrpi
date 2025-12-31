// modified library from HD44780_LCD_RDL by Gavin Lyons
// from Display_Lib_RPI
// Display_Lib_RPI is licensed under the MIT License

#include <hd44780.h>
#include <stdlib.h>
#include <stdio.h>
#include <msleep.h>

int main()
{
    HD44780 hd44780;

    msleep(500);

    if (!hd44780_init(&hd44780, 1, 0x27, 16, 2, LCDCursorTypeOn))
        return EXIT_FAILURE;

    hd44780_clear_screen(&hd44780);
    hd44780_set_backlight(&hd44780, true);

    hd44780_goto(&hd44780, LCDLineNumberOne, 0);
    hd44780_write_string(&hd44780, "bla");
    hd44780_goto(&hd44780, LCDLineNumberTwo , 0);
    hd44780_write_string(&hd44780, "blie");
    hd44780_write_char(&hd44780, '!');

    msleep(1000);

    // cursorMoveTest();
    // scrollTest();
    // gotoTest();
    // clearLineTest();
    // cursorTest();
    // entryModeTest();
    // writeNumTest();
    // customChar();
    // vectorTest();
    // stdarrayTest();
    // backLightTest();
    // ClockDemo(DISPLAY_CLOCK);

    printf("Press ENTER to quit\n");
    getchar();

    hd44780_LCDDisplayON(&hd44780, false);
    hd44780_close(&hd44780);

    return EXIT_SUCCESS;
}

