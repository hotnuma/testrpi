#ifndef __HD44780_H__
#define __HD44780_H__

// modified library from HD44780_LCD_RDL by Gavin Lyons
// from Display_Lib_RPI
// Display_Lib_RPI is licensed under the MIT License

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct hd44780
{
    int file;

    int channel;
    uint8_t addr;
    uint8_t cols;
    uint8_t rows;

    uint8_t backlight;

    uint16_t _I2C_ErrorDelay;   // retry delay in event of error in ms
    uint8_t _I2C_ErrorRetryNum; // number of retry attempts
    int _I2C_ErrorFlag;         // error code

    //int _LCDI2CFlags;
    //int _LCDI2CHandle;

} HD44780;

// backlight control
#define LCDBackLightOnMask      0x0F    // XXXX-1111 Turn on Back light
#define LCDBackLightOffMask     0x07    // XXXX-0111 Turn off Back light

// entry mode control
#define LCDEntryModeOne         0x04    // display shift :OFF decrement address counter
#define LCDEntryModeTwo         0x05    // display shift :ON  decrement address counter
#define LCDEntryModeThree       0x06    // display shift :OFF increment address counter, default
#define LCDEntryModeFour        0x07    // display shift :ON  increment address counter

// cursor mode
#define LCDCursorTypeOff        0x0C    // Make cursor invisible
#define LCDCursorTypeBlink      0x0D    // Turn on blinking-block cursor
#define LCDCursorTypeOn         0x0E    // Turn on visible  underline cursor
#define LCDCursorTypeOnBlink    0x0F    // Turn on blinking-block cursor + visible underline cursor

// direction mode for scroll and move
#define LCDMoveRight            1       // move or scroll right
#define LCDMoveLeft             2       // move or scroll left

#define LCDLineNumberOne        1
#define LCDLineNumberTwo        2
#define LCDLineNumberThree      3
#define LCDLineNumberFour       4


bool hd44780_init(HD44780 *hd44780, int channel, int addr, int cols, int rows);

inline ssize_t hd44780_write(HD44780 *hd44780, const void *data, int len)
{
    return write(hd44780->file, data, len);
}

void hd44780_LCDClearLine(HD44780 *hd44780, int line);

#if 0
void LCDInit(LCDCursorType_e);
void LCDDisplayON(bool);
void LCDResetScreen(LCDCursorType_e);

void LCDBackLightSet(bool);
bool LCDBackLightGet(void);

int LCD_I2C_ON(void);
int LCD_I2C_OFF(void);
int LCDCheckConnection(void);
int LCDI2CErrorGet(void);
uint16_t LCDI2CErrorTimeoutGet(void);
void LCDI2CErrorTimeoutSet(uint16_t);
uint8_t LCDI2CErrorRetryNumGet(void);
void LCDI2CErrorRetryNumSet(uint8_t);

int LCDSendString (char *str);
void LCDSendChar (char data);
virtual size_t write(uint8_t);
int  LCDCreateCustomChar(uint8_t location, uint8_t* charmap);
void LCDPrintCustomChar(uint8_t location);

void LCDMoveCursor(LCDDirectionType_e, uint8_t moveSize);
void LCDScroll(LCDDirectionType_e, uint8_t ScrollSize);
void LCDGOTO(LCDLineNumber_e  lineNo, uint8_t  col);
int  LCDClearScreen(void);
void LCDClearScreenCmd(void);
void LCDHome(void);
void LCDChangeEntryMode(LCDEntryMode_e mode);
#endif

#endif

