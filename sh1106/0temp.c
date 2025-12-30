
#if 0

inline bool i2c_test(int file, unsigned char addr)
{
    if (ioctl(file, I2C_SLAVE, addr) < 0)
        return false;

    unsigned char ucTemp;

    return (read(file, &ucTemp, 1) >= 0);
}

inline bool i2c_read(int file, unsigned char addr, unsigned char *data, int len)
{
    if (ioctl(file, I2C_SLAVE, addr) < 0)
        return false;

    return (read(file, data, len) > 0);
}

inline bool i2c_write(int file, unsigned char addr, unsigned char *data, int len)
{
    if (ioctl(file, I2C_SLAVE, addr) < 0)
        return false;

    return (write(file, data, len) >= 0);
}

// wrapper/adapter functions to make the code work on Linux
static uint8_t _pgm_read_byte(uint8_t *ptr)
{
    return *ptr;
}

static int16_t _pgm_read_word(uint8_t *ptr)
{
    return ptr[0] + (ptr[1] << 8);
}

// Read N bytes starting at a specific I2C internal register
// returns 1 for success, 0 for error
//
int I2CReadRegister(int file_i2c, unsigned char iAddr,
                    unsigned char u8Register, unsigned char *pData, int iLen)
{
    int i = 0;

    if (ioctl(file_i2c, I2C_SLAVE, iAddr) >= 0)
    {
        write(file_i2c, &u8Register, 1);
        i = read(file_i2c, pData, iLen);
    }

    return (i > 0);
}


// Detect the display controller (SSD1306, SH1107 or SH1106)
uint8_t u = 0;
I2CReadRegister(&pOLED->bbi2c, pOLED->oled_addr, 0x00, &u, 1); // read the status register
u &= 0x0f; // mask off power on/off bit
if (u == 0x7 || u == 0xf) // SH1107
{
    pOLED->oled_type = OLED_128x128;
    rc = OLED_SH1107_3C;
    bFlip = !bFlip; // SH1107 seems to have this reversed from the usual direction
}
else if (u == 0x8) // SH1106
{
    rc = OLED_SH1106_3C;
    pOLED->oled_type = OLED_132x64; // needs to be treated a little differently
}
else if (u == 3 || u == 6) // 6=128x64 display, 3=smaller
{
    printf("bla\n");
    rc = OLED_SSD1306_3C;
}



find the device address if requested
if (addr == -1 || addr == 0 || addr == 0xff)
{
    i2c_test(oled->file, 0x3c);

    if (i2c_test(oled->file, 0x3c))
        oled->addr = 0x3c;
    else if (i2c_test(oled->file, 0x3d))
        oled->addr = 0x3d;
    else
        return OLED_NOT_FOUND;
}
else
{
    oled->addr = addr;

    i2c_test(oled->file, addr);

    if (!i2c_test(oled->file, addr))
        return OLED_NOT_FOUND;
}


#endif

