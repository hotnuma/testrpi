// modified library from HD44780_LCD_RDL by Gavin Lyons
// from Display_Lib_RPI
// Display_Lib_RPI is licensed under the MIT License

#include <hd44780.h>
#include <libi2c.h>

// DDRAM address's used to set cursor position
#define LCDLineAddressOne       0x80 // Line 1
#define LCDLineAddressTwo       0xC0 // Line 2
#define LCDLineAddress3Col20    0x94 // Line 3 20x04 line 3
#define LCDLineAddress4Col20    0xD4 // Line 4 20x04 line 4
#define LCDLineAddress3Col16    0x90 // Line 3 16x04  untested, no part
#define LCDLineAddress4Col16    0xD0 // Line 4 16x04 untested, no part

// Command Bytes General
#define LCDCmdModeFourBit       0x28 // Function set (4-bit interface, 2 lines, 5*7 Pixels)
#define LCDCmdHomePosition      0x02 // Home (move cursor to top/left character position)
#define LCDCmdDisplayOn         0x0C // Restore the display (with cursor hidden)
#define LCDCmdDisplayOff        0x08 // Blank the display (without clearing)
#define LCDCmdClearScreen       0x01 // clear screen command byte

static void _hd44780_write_command(HD44780 *hd44780, unsigned char cmd);
static void _hd44780_write_data(HD44780 *hd44780, unsigned char data);

bool hd44780_init(HD44780 *hd44780, int channel, int addr, int cols, int rows)
{
    hd44780->channel = channel;
    hd44780->addr = addr;
    hd44780->cols = cols;
    hd44780->rows = rows;

    hd44780->backlight = LCDBackLightOnMask;

    hd44780->_I2C_ErrorDelay = 100;
    hd44780->_I2C_ErrorRetryNum = 3;
    hd44780->_I2C_ErrorFlag = 0;

    hd44780->file = i2c_init(channel, addr);

    if (hd44780->file < 0)
        return false;

    return true;
}

static void _hd44780_write_command(HD44780 *hd44780, unsigned char cmd)
{
    // I2C MASK Byte = COMD-led-en-rw-rs (en=enable rs = reg select) (rw always write)

    const uint8_t LCDCmdByteOn = 0x0C;  // enable=1 and rs =0 1100 COMD-led-en-rw-rs
    const uint8_t LCDCmdByteOff = 0x08; // enable=0 and rs =0 1000 COMD-led-en-rw-rs

    unsigned char nibble_upper = cmd & 0xf0;        // select upper nibble
    unsigned char nibble_lower = (cmd << 4) & 0xf0; // select lower nibble

    char buffer[4];

    buffer[0] = nibble_upper | (LCDCmdByteOn  & hd44780->backlight); // YYYY-1100 YYYY-led-en-rw-rs ,enable=1 and rs =0
    buffer[1] = nibble_upper | (LCDCmdByteOff & hd44780->backlight); // YYYY-1000 YYYY-led-en-rw-rs ,enable=0 and rs =0
    buffer[2] = nibble_lower | (LCDCmdByteOn  & hd44780->backlight); // YYYY-1100 YYYY-led-en-rw-rs ,enable=1 and rs =0
    buffer[3] = nibble_lower | (LCDCmdByteOff & hd44780->backlight); // YYYY-1000 YYYY-led-en-rw-rs ,enable=0 and rs =0

    /*ssize_t ErrorCode =*/ hd44780_write(hd44780, buffer, 4);

    // Error handling retransmit
    // uint8_t AttemptCount = hd44780->_I2C_ErrorRetryNum;
    // while (ErrorCode < 0)
    // {
    //     if (rdlib_config::isDebugEnabled())
    //     {
    //         fprintf(stderr, "Error:  _hd44780_write_command I2C: (%s) \n", lguErrorText(ErrorCode) );
    //         fprintf(stderr, "Attempt Count: %u \n", AttemptCount );
    //     }
    //     msleep(_I2C_ErrorDelay );
    //     ErrorCode = hd44780_write (hd44780, cmdBufferI2C, 4); // retransmit
    //     _I2C_ErrorFlag = ErrorCode;
    //     AttemptCount--;
    //     if (AttemptCount == 0) break;
    // }

    //_I2C_ErrorFlag = ErrorCode;
}

static void _hd44780_write_data(HD44780 *hd44780, unsigned char data)
{
    // I2C MASK Byte = DATA-led-en-rw-rs (en=enable rs = reg select)(rw always write)
    const uint8_t LCDDataByteOn = 0x0D;   // enable=1 and rs =1 1101 DATA-led-en-rw-rs
    const  uint8_t LCDDataByteOff = 0x09; // enable=0 and rs =1 1001 DATA-led-en-rw-rs

    char dataBufferI2C[4];

    unsigned char dataNibbleLower = (data << 4) & 0xf0; //select lower nibble by moving it to the upper nibble position
    unsigned char dataNibbleUpper = data & 0xf0; //select upper nibble
    dataBufferI2C[0] = dataNibbleUpper | (LCDDataByteOn & hd44780->backlight); //enable=1 and rs =1 1101  YYYY-X-en-X-rs
    dataBufferI2C[1] = dataNibbleUpper | (LCDDataByteOff & hd44780->backlight); //enable=0 and rs =1 1001 YYYY-X-en-X-rs
    dataBufferI2C[2] = dataNibbleLower | (LCDDataByteOn & hd44780->backlight); //enable=1 and rs =1 1101  YYYY-X-en-X-rs
    dataBufferI2C[3] = dataNibbleLower | (LCDDataByteOff &  hd44780->backlight); //enable=0 and rs =1 1001 YYYY-X-en-X-rs

    /*int ErrorCode =*/ hd44780_write(hd44780, dataBufferI2C, 4);

    // Error handling retransmit
    // uint8_t AttemptCount = hd44780->_I2C_ErrorRetryNum;
    // while (ErrorCode < 0)
    // {
    //         if (rdlib_config::isDebugEnabled())
    //         {
    //                 fprintf(stderr, "Error:  LCDSendData I2C: (%s) \n", lguErrorText(ErrorCode) );
    //                 fprintf(stderr, "Attempt Count: %u \n", AttemptCount );
    //         }
    //         msleep(_I2C_ErrorDelay );
    //         ErrorCode = hd44780_write (hd44780, dataBufferI2C, 4); // retransmit
    //         _I2C_ErrorFlag = ErrorCode;
    //         AttemptCount--;
    //         if (AttemptCount == 0) break;
    // }
    // _I2C_ErrorFlag = ErrorCode;
}

void hd44780_LCDClearLine(HD44780 *hd44780, int line)
{
    // clear a line by writing spaces to every position

    switch (line)
    {
        case LCDLineNumberOne:
            _hd44780_write_command(hd44780, LCDLineAddressOne);
            break;

        case LCDLineNumberTwo:
            _hd44780_write_command(hd44780, LCDLineAddressTwo);
            break;

        case LCDLineNumberThree:
            switch (hd44780->cols)
            {
                case 16:
                    _hd44780_write_command(hd44780, LCDLineAddress3Col16);
                    break;

                case 20:
                    _hd44780_write_command(hd44780, LCDLineAddress3Col20);
                    break;
            }
            break;

        case LCDLineNumberFour:
            switch (hd44780->cols)
            {
                case 16:
                _hd44780_write_command(hd44780, LCDLineAddress4Col16);
                    break;

                case 20:
                    _hd44780_write_command(hd44780, LCDLineAddress4Col20);
                    break;
            }
            break;
    }

    for (uint8_t i = 0; i < hd44780->cols; ++i)
    {
        _hd44780_write_data(hd44780, ' ');
    }
}

#if 0



/*!
	@brief  Clear screen by writing spaces to every position
	@note : See also LCDClearScreenCmd for software command clear alternative.
	@return 
		-#  Success
		-#  GenericError of number of rows invalid 

*/
 int hd44780_LCDClearScreen(HD44780 *hd44780) {
	if (_NumRowsLCD < 1 || _NumRowsLCD >4)
	{
		if (rdlib_config::isDebugEnabled())
		{
			fprintf(stderr, "Error: LCDClearScreen  : Number of rows invalid, must be: %u \n", _NumRowsLCD);
			return rdlib::GenericError ;
		}
	}

	LCDClearLine(LCDLineNumberOne);

	if (_NumRowsLCD >= 2)
		LCDClearLine(LCDLineNumberTwo);
	if (_NumRowsLCD >= 3)
		LCDClearLine(LCDLineNumberThree);
	if (_NumRowsLCD == 4)
		LCDClearLine(LCDLineNumberFour);
	
	return rdlib::Success;
}


/*!
	@brief  Reset screen
	@param CursorType LCDCursorType_e enum cursor type, 4 choices
*/
void hd44780_LCDResetScreen(HD44780 *hd44780, LCDCursorType_e CursorType) {
         _hd44780_write_command(hd44780, LCDCmdModeFourBit);
         _hd44780_write_command(hd44780, LCDCmdDisplayOn);
         _hd44780_write_command(hd44780, CursorType);
         _hd44780_write_command(hd44780, LCDCmdClearScreen);
         _hd44780_write_command(hd44780, LCDEntryModeThree);
        msleep(5);
}

/*!
	@brief  Turn Screen on and off
	@param OnOff  True = display on , false = display off
*/
void hd44780_LCDDisplayON(HD44780 *hd44780, bool OnOff) {
        OnOff ?  _hd44780_write_command(hd44780, LCDCmdDisplayOn) :  _hd44780_write_command(hd44780, LCDCmdDisplayOff);
        msleep(5);
}


/*!
	@brief  Initialise LCD
	@param CursorType  The cursor type 4 choices.
*/
void hd44780_LCDInit(HD44780 *hd44780, LCDCursorType_e CursorType) {

        msleep(15);
         _hd44780_write_command(hd44780, LCDCmdHomePosition);
        msleep(5);
         _hd44780_write_command(hd44780, LCDCmdHomePosition);
        msleep(5);
         _hd44780_write_command(hd44780, LCDCmdHomePosition);
        msleep(5);
         _hd44780_write_command(hd44780, LCDCmdModeFourBit);
         _hd44780_write_command(hd44780, LCDCmdDisplayOn);
         _hd44780_write_command(hd44780, CursorType);
         _hd44780_write_command(hd44780, LCDEntryModeThree);
         _hd44780_write_command(hd44780, LCDCmdClearScreen);
        msleep(5);
}

/*!
	@brief  Send a string to LCD
	@param str  Pointer to the char array
	@return codes
		-# Success
		-# CharArrayNullptr
		-# CustomCharLen
*/
int hd44780_LCDSendString(HD44780 *hd44780, char *str)
{
	// Check for null pointer
	if (str == nullptr) 
	{
		fprintf(stderr ,"Error: LCDSendString 1: String is a null pointer.\n");
		return rdlib::CharArrayNullptr;
	}
	// Check for correct length for screen size.
        size_t maxLength = static_cast<size_t>(hd44780->cols * _NumRowsLCD + 1);
	if (strlen(str) > maxLength)
	{
		fprintf(stderr, "Error: LCDSendString 2: String length is larger than screen.\n");
		return rdlib::CustomCharLen;
	}
        while (*str) _hd44780_write_data(hd44780, *str++);
	return rdlib::Success;
}


/*!
	@brief  Sends a character to screen , simply wraps SendData command.
	@param data Character to display
*/
void hd44780_LCDSendChar(HD44780 *hd44780, char data) {
        _hd44780_write_data(hd44780, data);
}


/*!
	@brief  Moves cursor
	@param direction enum LCDDirectionType_e left or right
	@param moveSize number of spaces to move
*/
void hd44780_LCDMoveCursor(HD44780 *hd44780, LCDDirectionType_e direction, uint8_t moveSize) {
	uint8_t i = 0;
	const uint8_t LCDMoveCursorLeft = 0x10;  //Command Byte Code:  Move cursor one character left
	const uint8_t LCDMoveCursorRight = 0x14;  // Command Byte Code : Move cursor one character right
	switch(direction)
	{
	case LCDMoveRight:
		for (i = 0; i < moveSize; i++) {
                         _hd44780_write_command(hd44780, LCDMoveCursorRight);
		}
	break;
	case LCDMoveLeft:
		for (i = 0; i < moveSize; i++) {
                         _hd44780_write_command(hd44780, LCDMoveCursorLeft);
		}
	break;
	}
}

/*!
	@brief  Scrolls screen
	@param direction  left or right
	@param ScrollSize number of spaces to scroll
*/
void hd44780_LCDScroll(HD44780 *hd44780, LCDDirectionType_e direction, uint8_t ScrollSize) {
	uint8_t i = 0;

	const uint8_t LCDScrollRight = 0x1E;  // Command Byte Code: Scroll display one character right (all lines)
	const uint8_t LCDScrollLeft = 0x18;  //Command Byte Code: Scroll display one character left (all lines)

	switch(direction)
	{
	case LCDMoveRight:
		for (i = 0; i < ScrollSize; i++) {
                         _hd44780_write_command(hd44780, LCDScrollRight);
		}
	break;
	case LCDMoveLeft:
		for (i = 0; i < ScrollSize; i++) {
                         _hd44780_write_command(hd44780, LCDScrollLeft);
		}
	break;
	}
}

/*!
	@brief  moves cursor to an x , y position on display.
	@param  line  x row 1-4
	@param col y column  0-15 or 0-19
*/
void hd44780_LCDGOTO(HD44780 *hd44780, LCDLineNumber_e line, uint8_t col) {

	switch (line) {
                case LCDLineNumberOne:  _hd44780_write_command(hd44780, LCDLineAddressOne| col); break;
                case LCDLineNumberTwo:  _hd44780_write_command(hd44780, LCDLineAddressTwo | col); break;
		case LCDLineNumberThree:
                        switch (hd44780->cols)
			{
                                case 16:  _hd44780_write_command(hd44780, LCDLineAddress3Col16 | col); break;
                                case 20:  _hd44780_write_command(hd44780, LCDLineAddress3Col20 + col); break;
			}
		break;
		case LCDLineNumberFour:
                        switch (hd44780->cols)
			{
                                case 16:  _hd44780_write_command(hd44780, LCDLineAddress4Col16 | col); break;
                                case 20:  _hd44780_write_command(hd44780, LCDLineAddress4Col20 + col); break;
			}
		break;
	}
}

/*!
	@brief  Saves a custom character to a location in character generator RAM 64 bytes.
	@param location CG_RAM location 0-7, we only have 8 locations 64 bytes
	@param charmap An array of 8 bytes representing a custom character data
	@return codes
		-# Success
		-# CharArrayNullptr
		-# InvalidRAMLocation
*/
int  hd44780_LCDCreateCustomChar(HD44780 *hd44780, uint8_t location, uint8_t * charmap)
{
	if (charmap == nullptr) 
	{
		fprintf(stderr ,"Error:LCDCreateCustomChar 1: character map is a null pointer.\n");
		return rdlib::CharArrayNullptr;
	}
	if (location >= 8) {
		fprintf(stderr ,"Error:LCDCreateCustomChar 2: CG_RAM location invalid  must be 0-7 \n");
		return rdlib::InvalidRAMLocation;
	}
	const uint8_t LCD_CG_RAM = 0x40;  //  character-generator RAM (CG RAM address)
         _hd44780_write_command(hd44780, LCD_CG_RAM | (location<<3));
	for (uint8_t i=0; i<8; i++) {
                _hd44780_write_data(hd44780, charmap[i]);
	}
	return rdlib::Success;
}

/*!
	@brief  Turn LED backlight on and off
	@param OnOff passed bool True = LED on , false = display LED off
	@note another data or command must be issued before it takes effect.
*/
void hd44780_LCDBackLightSet(HD44780 *hd44780, bool OnOff)
{
         OnOff ? (hd44780->backlight= LCDBackLightOnMask) : (hd44780->backlight= LCDBackLightOffMask);
}

/*!
	@brief  get the backlight flag status
	@return the status of backlight on or off , true or false.
*/

bool hd44780_LCDBackLightGet(HD44780 *hd44780)
{
        switch(hd44780->backlight){
		case LCDBackLightOnMask : return true; break;
		case LCDBackLightOffMask: return false; break;
		default : return true ; break ;
	}
}

/*!
	@brief Switch on the I2C
	@note Start I2C operations. 
	@return error for failure to init a I2C bus
		-# rdlib::Success
		-# rdlib::I2CbeginFail
*/
int hd44780_LCD_I2C_ON(HD44780 *hd44780)
{
	int I2COpenHandle = 0;

	I2COpenHandle = Display_RDL_I2C_OPEN(_LCDI2CDevice, _LCDI2CAddress, _LCDI2CFlags);
	if (I2COpenHandle < 0 )
	{
		fprintf(stderr, "Error: LCD_I2C_ON: Can't open I2C (%s) \n", lguErrorText(I2COpenHandle));
		return rdlib::I2CbeginFail;
	}
	else
	{
		_LCDI2CHandle = I2COpenHandle;
		return rdlib::Success;
	}
}


/*!
	@brief End I2C operations
	@return error for failure to close a I2C bus device
		-# rdlib::Success
		-# rdlib::I2CcloseFail
*/
int hd44780_LCD_I2C_OFF(HD44780 *hd44780)
{
	int I2CCloseHandleStatus = 0;

	I2CCloseHandleStatus = Display_RDL_I2C_CLOSE(_LCDI2CHandle);
	if (I2CCloseHandleStatus < 0 )
	{
		fprintf(stderr,  "Error:  LCD_I2C_OFF : Can't Close I2C (%s) \n", lguErrorText(I2CCloseHandleStatus));
		return rdlib::I2CcloseFail;
	}
	return rdlib::Success;
}

/*!
	@brief Print out a customer character from character generator CGRAM 64 bytes 8 characters
	@param location CGRAM  0-7
*/
void hd44780_LCDPrintCustomChar(HD44780 *hd44780, uint8_t location)
{
	if (location >= 8) {return;}
        _hd44780_write_data(hd44780, location);
}

/*!
	@brief  Called by print class, used to print out numerical data types etc
	@param character write a character
	@note used internally. Called by the print method using virtual
	@return returns 1 to the print class
*/
size_t hd44780_write(HD44780 *hd44780, uint8_t character)
{
	LCDSendChar(character) ;
	return 1;
}

/*!
	@brief Clear display using software command , set cursor position to zero
	@note  See also LCDClearScreen for manual clear
*/
void hd44780_LCDClearScreenCmd(HD44780 *hd44780) {
         _hd44780_write_command(hd44780, LCDCmdClearScreen);
        msleep(3); // Requires a delay
}

/*!
	@brief Set cursor position to home position .
*/
void hd44780_LCDHome(HD44780 *hd44780) {
         _hd44780_write_command(hd44780, LCDCmdHomePosition);
        msleep(3); // Requires a delay
}

/*!
	@brief Change entry mode
	@param newEntryMode  1-4 , 4 choices.
*/
void hd44780_LCDChangeEntryMode(HD44780 *hd44780, LCDEntryMode_e newEntryMode)
{
         _hd44780_write_command(hd44780, newEntryMode);
        msleep(3); // Requires a delay
}


/*!
	@brief get I2C error Flag
	@details See Error Codes at bottom of https://abyz.me.uk/lg/lgpio.html
	@return I2C error flag
*/
int hd44780_LCDI2CErrorGet(HD44780 *hd44780) { return _I2C_ErrorFlag;}

/*!
	 @brief Sets the I2C timeout, in the event of an I2C write error
	 @details Delay between retry attempts in event of an error , mS
	 @param newTimeout I2C timeout delay in mS
*/
void hd44780_LCDI2CErrorTimeoutSet(HD44780 *hd44780, uint16_t newTimeout)
{
	_I2C_ErrorDelay = newTimeout;
}

/*!
	 @brief Gets the I2C timeout, used in the event of an I2C write error
	 @details Delay between retry attempts in event of an error , mS
	 @return  I2C timeout delay in mS, _I2C_ErrorDelay
*/
uint16_t hd44780_LCDI2CErrorTimeoutGet(HD44780 *hd44780){return _I2C_ErrorDelay;}

/*!
	 @brief Gets the I2C error retry attempts, used in the event of an I2C write error
	 @details Number of times to retry in event of an error
         @return   hd44780->_I2C_ErrorRetryNum
*/
uint8_t hd44780_LCDI2CErrorRetryNumGet(HD44780 *hd44780){return hd44780->_I2C_ErrorRetryNum;}

/*!
	 @brief Sets the I2C error retry attempts used in the event of an I2C write error
	 @details Number of times to retry in event of an error
	 @param AttemptCount I2C retry attempts
*/
void hd44780_LCDI2CErrorRetryNumSet(HD44780 *hd44780, uint8_t AttemptCount)
{
        hd44780->_I2C_ErrorRetryNum = AttemptCount;
}


/*!
	@brief checks if LCD on I2C bus
	@return lg Error codes, LG_OKAY   0x00 = Success
	@note Error codes are here https://abyz.me.uk/lg/lgpio.html
		prints error code text to console
*/
int hd44780_LCDCheckConnection(HD44780 *hd44780)
{
	char rxdatabuf[1]; //buffer to hold return byte
	int I2CReadStatus = 0;

        I2CReadStatus = Display_RDL_I2C_READ(hd44780, rxdatabuf, 1);
	if (I2CReadStatus < 0 )
	{
		fprintf(stderr, "Error: LCDCheckConnection :Cannot read device (%s)\n",lguErrorText(I2CReadStatus));
	}

	_I2C_ErrorFlag = I2CReadStatus;
	return _I2C_ErrorFlag;
}

#endif

