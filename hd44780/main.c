// modified library from HD44780_LCD_RDL by Gavin Lyons
// from Display_Lib_RPI
// Display_Lib_RPI is licensed under the MIT License

#include <hd44780.h>
#include <stdlib.h>
#include <msleep.h>

int main()
{
    HD44780 hd44780;

    if (!hd44780_init(&hd44780, 1, 0x27, 16, 2))
        return EXIT_FAILURE;

    // if (!myLCD.LCD_I2C_ON())
    //     return EXIT_FAILURE;

    msleep(500);

    // if (myLCD.LCDCheckConnection() < 0)// check on bus  ( Note optional)
    // {
    //     std::cout << "Error 1202: LCD not on bus?" << std::endl;
    //     return false;
    // }else {
    //     std::cout << "LCDCheckConnection passed : LCD detected on the I2C bus" << std::endl;
    // }

    // myLCD.LCDInit(myLCD.LCDCursorTypeOn);
    // myLCD.LCDClearScreen();
    // myLCD.LCDBackLightSet(true);

    //        // Print out flag status ( Note optional)
    // std::cout << "Backlight status is : " << (myLCD.LCDBackLightGet() ? "On" : "Off") << std::endl ;
    // std::cout << "I2C Debug Error : " << myLCD.LCDI2CErrorGet() << std::endl; // Print I2C error flag
    // std::cout << "I2C Error Timeout mS : " << myLCD.LCDI2CErrorTimeoutGet() << std::endl; // Print I2C error Timeout
    // std::cout << "I2C Error retry attempts counts: " << +myLCD.LCDI2CErrorRetryNumGet() << std::endl; // Print I2C error retry count
    // std::cout << "Debug enabled : " << (rdlib_config::isDebugEnabled() ? "On" : "Off") << std::endl ;

    // helloWorld();
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

    // endTest();

    return EXIT_SUCCESS;
}

