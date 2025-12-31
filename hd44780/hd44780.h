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


bool hd44780_init(HD44780 *hd44780, int channel, int addr,
                  int cols, int rows, uint8_t cursor_type);
void hd44780_close(HD44780 *hd44780);

inline ssize_t hd44780_write(HD44780 *hd44780, const void *data, int len)
{
    return write(hd44780->file, data, len);
}

void hd44780_clear_line(HD44780 *hd44780, int line);
bool hd44780_clear_screen(HD44780 *hd44780);
void hd44780_LCDResetScreen(HD44780 *hd44780, uint8_t cursor_type);
void hd44780_LCDDisplayON(HD44780 *hd44780, bool on);
bool hd44780_write_string(HD44780 *hd44780, char *str);
void hd44780_write_char(HD44780 *hd44780, char data);
void hd44780_LCDMoveCursor(HD44780 *hd44780, uint8_t direction, uint8_t num);
void hd44780_LCDScroll(HD44780 *hd44780, uint8_t direction, uint8_t ScrollSize);
void hd44780_goto(HD44780 *hd44780, uint8_t line, uint8_t col);
void hd44780_set_backlight(HD44780 *hd44780, bool on);
bool hd44780_LCDBackLightGet(HD44780 *hd44780);
void hd44780_LCDPrintCustomChar(HD44780 *hd44780, uint8_t location);
void hd44780_LCDClearScreenCmd(HD44780 *hd44780);
void hd44780_LCDHome(HD44780 *hd44780);
void hd44780_LCDChangeEntryMode(HD44780 *hd44780, uint8_t newEntryMode);

int hd44780_LCDI2CErrorGet(HD44780 *hd44780);
void hd44780_LCDI2CErrorTimeoutSet(HD44780 *hd44780, uint16_t newTimeout);
uint16_t hd44780_LCDI2CErrorTimeoutGet(HD44780 *hd44780);
uint8_t hd44780_LCDI2CErrorRetryNumGet(HD44780 *hd44780);
void hd44780_LCDI2CErrorRetryNumSet(HD44780 *hd44780, uint8_t AttemptCount);
int hd44780_LCDCheckConnection(HD44780 *hd44780);

#endif

