// ss_oled (Small, Simple OLED library)
// Copyright (c) 2017-2019 BitBank Software, Inc.
// Written by Larry Bank (bitbank@pobox.com)
// Project started 1/15/2017
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "ss_oled.h"
#include "i2c_funcs.h"
#include "global.h"

#include <stdlib.h>
#include <string.h>

// initialization sequences
const unsigned char oled32_initbuf[] =
{
    0x00,0xae,0xd5,0x80,0xa8,0x1f,0xd3,0x00,0x40,0x8d,0x14,0xa1,0xc8,0xda,0x02,
    0x81,0x7f,0xd9,0xf1,0xdb,0x40,0xa4,0xa6,0xaf
};

const unsigned char oled64_initbuf[] =
{
    0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,
    0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,
    0xaf,0x20,0x02
};

const unsigned char oled72_initbuf[] =
{
    0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,
    0xda,0x12,0x81,0xff,0xad,0x30,0xd9,0xf1,0xa4,0xa6,0xd5,0x80,0x8d,0x14,
    0xaf,0x20,0x02
};

const unsigned char oled128_initbuf[] =
{
    0x00, 0xae,0xdc,0x00,0x81,0x40,
    0xa1,0xc8,0xa8,0x7f,0xd5,0x50,0xd9,0x22,0xdb,0x35,0xb0,0xda,0x12,
    0xa4,0xa6,0xaf
};

static void _oled_write_command(SSOLED *oled, unsigned char c);
static void _oled_write_command2(SSOLED *oled, unsigned char c, unsigned char d);

static void _oled_set_position(SSOLED *oled, int x, int y, bool render);
static void _oled_write_datablock(SSOLED *oled, unsigned char *buffer, int len, bool render);
static void _invert_bytes(uint8_t *data, uint8_t len);

static void _oled_write_command(SSOLED *oled, unsigned char c)
{
    unsigned char buf[2];
    buf[0] = 0x00;

    buf[1] = c;
    oled_write(oled, buf, 2);
}

static void _oled_write_command2(SSOLED *oled, unsigned char c, unsigned char d)
{
    unsigned char buf[3];
    buf[0] = 0x00;

    buf[1] = c;
    buf[2] = d;
    oled_write(oled, buf, 3);
}

int oled_init(SSOLED *oled, int channel, int addr,
              int type, int res,
              bool flip, bool invert)
{
    unsigned char uc[4];

    oled->screen = NULL; // reset backbuffer; user must provide one later
    oled->res = res;
    oled->flip = flip;
    oled->wrap = false; // default - disable text wrap

    oled->file = i2c_init(channel, addr);

    if (oled->file == -1)
        return OLED_NOT_FOUND;

    oled->addr = addr;
    
    // SH1106 is 128 centered in 132
    if (type == OLED_SH1106)
        oled->res = OLED_132x64;

    if (res == OLED_128x32 || res == OLED_96x16)
        oled_write(oled, (unsigned char*) oled32_initbuf, sizeof(oled32_initbuf));
    else if (res == OLED_128x128)
        oled_write(oled, (unsigned char*) oled128_initbuf, sizeof(oled128_initbuf));
    else if (res == OLED_72x40)
        oled_write(oled, (unsigned char*) oled72_initbuf, sizeof(oled72_initbuf));
    else // 132x64, 128x64 and 64x32
        oled_write(oled, (unsigned char*) oled64_initbuf, sizeof(oled64_initbuf));
    
    if (invert)
    {
        uc[0] = 0;

        // invert command
        uc[1] = 0xa7;
        oled_write(oled,uc, 2);
    }

    if (flip)
    {
        uc[0] = 0;

       // rotate display 180
        uc[1] = 0xa0;
        oled_write(oled,uc, 2);

        uc[1] = 0xc0;
        oled_write(oled,uc, 2);
    }

    if (res == OLED_96x16)
    {
        oled->oled_x = 96;
        oled->oled_y = 16;
    }
    else if (res == OLED_128x32)
    {
        oled->oled_x = 128;
        oled->oled_y = 32;
    }
    else if (res == OLED_128x128)
    {
        oled->oled_x = 128;
        oled->oled_y = 128;
    }
    else if (res == OLED_64x32)
    {
        oled->oled_x = 64;
        oled->oled_y = 32;
    }
    else if (res == OLED_72x40)
    {
        oled->oled_x = 72;
        oled->oled_y = 40;
    }
    else
    {
        oled->oled_x = 128;
        oled->oled_y = 64;
    }
    
    return type;
}

void oled_set_backbuffer(SSOLED *oled, uint8_t *buffer)
{
    oled->screen = buffer;
}

void oled_fill(SSOLED *oled, unsigned char data, int render)
{
    unsigned char temp[16];
    memset(temp, data, 16);

    oled->cursor_x = 0;
    oled->cursor_y = 0;

    uint8_t lines = oled->oled_y >> 3;
    uint8_t cols = oled->oled_x >> 4;

    for (uint8_t y = 0; y < lines; ++y)
    {
        _oled_set_position(oled, 0, y, render);

        // wiring library has a 32-byte buffer,
        // so send 16 bytes so that the data prefix (0x40) can fit
        for (uint8_t x = 0; x < cols; ++x)
        {
            _oled_write_datablock(oled, temp, 16, render);
        }

        // 72 isn't evenly divisible by 16, so fix it
        if (oled->res == OLED_72x40)
            _oled_write_datablock(oled, temp, 8, render);
    }

    if (oled->screen)
        memset(oled->screen, data, (oled->oled_x * oled->oled_y) / 8);
}

static void _oled_set_position(SSOLED *oled, int x, int y, bool render)
{
    // send commands to position the "cursor" (aka memory write address)
    // to the given row and column

    unsigned char buf[4];

    oled->screen_offset = (y*128) + x;

    if (!render)
        return;

    if (oled->res == OLED_64x32) // visible display starts at column 32, row 4
    {
        x += 32; // display is centered in VRAM, so this is always true
        if (oled->flip == 0) // non-flipped display starts from line 4
            y += 4;
    }
    else if (oled->res == OLED_132x64) // SH1106 has 128 pixels centered in 132
    {
        x += 2;
    }
    else if (oled->res == OLED_96x16) // visible display starts at line 2
    {
        // mapping is a bit strange on the 96x16 OLED
        if (oled->flip)
            x += 32;
        else
            y += 2;
    }
    else if (oled->res == OLED_72x40) // starts at x=28,y=3
    {
        x += 28;
        if (!oled->flip)
        {
            y += 3;
        }
    }

    buf[0] = 0x00;

    buf[1] = 0xb0 | y; // set page to Y
    buf[2] = x & 0xf; // lower column address
    buf[3] = 0x10 | (x >> 4); // upper column addr

    oled_write(oled, buf, 4);
}

static void _oled_write_datablock(SSOLED *oled, unsigned char *buffer, int len, bool render)
{
    // write a block of pixel data to the OLED
    // length can be anything from 1 to 1024 (whole display)

    unsigned char temp[129];

    temp[0] = 0x40; // data command

    // copying the data has the benefit in SPI mode of not letting
    // the original data get overwritten by the SPI.transfer() function
    if (render)
    {
        memcpy(&temp[1], buffer, len);
        oled_write(oled, temp, len+1);
    }

    // keep a copy in local buffer
    if (oled->screen)
    {
        memcpy(&oled->screen[oled->screen_offset], buffer, len);
        oled->screen_offset += len;

        // we use a fixed stride of 128 no matter what the display size
        oled->screen_offset &= 1023;
    }
}

void oled_power(SSOLED *oled, bool on)
{
    if (on)
        _oled_write_command(oled, 0xaf);
    else
        _oled_write_command(oled, 0xae);
}

void oled_set_contrast(SSOLED *oled, unsigned char contrast)
{
    // (0=off, 255=brightest)
    _oled_write_command2(oled, 0x81, contrast);
}

//
// Set the current cursor position
// The column represents the pixel column (0-127)
// The row represents the text row (0-7)
//
void oled_set_cursor(SSOLED *oled, int x, int y)
{
    oled->cursor_x = x;
    oled->cursor_y = y;
}

void oled_set_textwrap(SSOLED *oled, int wrap)
{
    // turn text wrap on or off for the oldWriteString() function

    oled->wrap = wrap;
}

int oled_string_write(SSOLED *oled, int scroll, int x, int y,
                      char *msg, int size, bool invert, bool render)
{
    // draw a string of normal (8x8), small (6x8) or large (16x32) characters

    int i;
    int iFontOff;
    int iLen;
    int iFontSkip;
    unsigned char c;
    unsigned char *s;
    unsigned char ucTemp[40];

    if (x == -1 || y == -1) // use the cursor position
    {
        x = oled->cursor_x; y = oled->cursor_y;
    }
    else
    {
        oled->cursor_x = x; oled->cursor_y = y; // set the new cursor position
    }

    if ((oled->cursor_x >= oled->oled_x) || (oled->cursor_y >= (oled->oled_y / 8)))
        return -1; // can't draw off the display

    _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);

    // 8x8 font
    if (size == FONT_8x8)
    {
        i = 0;
        iFontSkip = scroll & 7; // number of columns to initially skip
        while (oled->cursor_x < oled->oled_x && msg[i] != 0 && oled->cursor_y < oled->oled_y / 8)
        {
            if (scroll < 8) // only display visible characters
            {
                c = (unsigned char)msg[i];
                iFontOff = (int)(c-32) * 7;
                // we can't directly use the pointer to FLASH memory, so copy to a local buffer
                ucTemp[0] = 0;
                memcpy_P(&ucTemp[1], &ucFont[iFontOff], 7);
                if (invert) _invert_bytes(ucTemp, 8);
                //         oledCachedWrite(ucTemp, 8);
                iLen = 8 - iFontSkip;
                if (oled->cursor_x + iLen > oled->oled_x) // clip right edge
                    iLen = oled->oled_x - oled->cursor_x;
                _oled_write_datablock(oled, &ucTemp[iFontSkip], iLen, render); // write character pattern
                oled->cursor_x += iLen;
                if (oled->cursor_x >= oled->oled_x-7 && oled->wrap) // word wrap enabled?
                {
                    oled->cursor_x = 0; // start at the beginning of the next line
                    oled->cursor_y++;
                    _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);
                }
                iFontSkip = 0;
            }
            scroll -= 8;
            i++;
        }
        // while
        //     oledCachedFlush(); // write any remaining data
        return 0;
    } // 8x8

    else if (size == FONT_16x32) // 16x32 font
    {
        i = 0;
        iFontSkip = scroll & 15; // number of columns to initially skip
        while (oled->cursor_x < oled->oled_x && oled->cursor_y < (oled->oled_y / 8)-3 && msg[i] != 0)
        {
            if (scroll < 16) // if characters are visible
            {
                s = (unsigned char *)&ucBigFont[(unsigned char)(msg[i]-32)*64];
                iLen = 16 - iFontSkip;
                if (oled->cursor_x + iLen > oled->oled_x) // clip right edge
                    iLen = oled->oled_x - oled->cursor_x;
                // we can't directly use the pointer to FLASH memory, so copy to a local buffer
                _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);
                memcpy_P(ucTemp, s, 16);
                if (invert) _invert_bytes(ucTemp, 16);
                _oled_write_datablock(oled, &ucTemp[iFontSkip], iLen, render); // write character pattern
                _oled_set_position(oled, oled->cursor_x, oled->cursor_y+1, render);
                memcpy_P(ucTemp, s+16, 16);
                if (invert) _invert_bytes(ucTemp, 16);
                _oled_write_datablock(oled, &ucTemp[iFontSkip], iLen, render); // write character pattern
                if (oled->cursor_y <= 5)
                {
                    _oled_set_position(oled, oled->cursor_x, oled->cursor_y+2, render);
                    memcpy_P(ucTemp, s+32, 16);
                    if (invert) _invert_bytes(ucTemp, 16);
                    _oled_write_datablock(oled, &ucTemp[iFontSkip], iLen, render); // write character pattern
                }
                if (oled->cursor_y <= 4)
                {
                    _oled_set_position(oled, oled->cursor_x, oled->cursor_y+3, render);
                    memcpy_P(ucTemp, s+48, 16);
                    if (invert) _invert_bytes(ucTemp, 16);
                    _oled_write_datablock(oled, &ucTemp[iFontSkip], iLen, render); // write character pattern
                }
                oled->cursor_x += iLen;
                if (oled->cursor_x >= oled->oled_x-15 && oled->wrap) // word wrap enabled?
                {
                    oled->cursor_x = 0; // start at the beginning of the next line
                    oled->cursor_y+=4;
                }
                iFontSkip = 0;
            } // if character visible from scrolling
            scroll -= 16;
            i++;
        } // while
        return 0;
    } // 16x32

    else if (size == FONT_12x16) // 6x8 stretched to 12x16
    {
        i = 0;
        iFontSkip = scroll % 12; // number of columns to initially skip
        while (oled->cursor_x < oled->oled_x && oled->cursor_y < (oled->oled_y/8)-1 && msg[i] != 0)
        {
            // stretch the 'normal' font instead of using the big font
            if (scroll < 12) // if characters are visible
            {
                int tx, ty;
                c = msg[i] - 32;
                unsigned char uc1, uc2, ucMask, *pDest;
                s = (unsigned char *)&ucSmallFont[(int)c*5];
                ucTemp[0] = 0; // first column is blank
                memcpy_P(&ucTemp[1], s, 6);
                if (invert)
                    _invert_bytes(ucTemp, 6);
                // Stretch the font to double width + double height
                memset(&ucTemp[6], 0, 24); // write 24 new bytes
                for (tx=0; tx<6; tx++)
                {
                    ucMask = 3;
                    pDest = &ucTemp[6+tx*2];
                    uc1 = uc2 = 0;
                    c = ucTemp[tx];
                    for (ty=0; ty<4; ty++)
                    {
                        if (c & (1 << ty)) // a bit is set
                            uc1 |= ucMask;
                        if (c & (1 << (ty + 4)))
                            uc2 |= ucMask;
                        ucMask <<= 2;
                    }
                    pDest[0] = uc1;
                    pDest[1] = uc1; // double width
                    pDest[12] = uc2;
                    pDest[13] = uc2;
                }
                // smooth the diagonal lines
                for (tx=0; tx<5; tx++)
                {
                    uint8_t c0, c1, ucMask2;
                    c0 = ucTemp[tx];
                    c1 = ucTemp[tx+1];
                    pDest = &ucTemp[6+tx*2];
                    ucMask = 1;
                    ucMask2 = 2;
                    for (ty=0; ty<7; ty++)
                    {
                        if (((c0 & ucMask) && !(c1 & ucMask) && !(c0 & ucMask2) && (c1 & ucMask2)) || (!(c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask2) && !(c1 & ucMask2)))
                        {
                            if (ty < 3) // top half
                            {
                                pDest[1] |= (1 << ((ty * 2)+1));
                                pDest[2] |= (1 << ((ty * 2)+1));
                                pDest[1] |= (1 << ((ty+1) * 2));
                                pDest[2] |= (1 << ((ty+1) * 2));
                            }
                            else if (ty == 3) // on the border
                            {
                                pDest[1] |= 0x80; pDest[2] |= 0x80;
                                pDest[13] |= 1; pDest[14] |= 1;
                            }
                            else // bottom half
                            {
                                pDest[13] |= (1 << (2*(ty-4)+1));
                                pDest[14] |= (1 << (2*(ty-4)+1));
                                pDest[13] |= (1 << ((ty-3) * 2));
                                pDest[14] |= (1 << ((ty-3) * 2));
                            }
                        }
                        else if (!(c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask2) && !(c1 & ucMask2))
                        {
                            if (ty < 4) // top half
                            {
                                pDest[1] |= (1 << ((ty * 2)+1));
                                pDest[2] |= (1 << ((ty+1) * 2));
                            }
                            else
                            {
                                pDest[13] |= (1 << (2*(ty-4)+1));
                                pDest[14] |= (1 << ((ty-3) * 2));
                            }
                        }
                        ucMask <<= 1; ucMask2 <<= 1;
                    }
                }
                iLen = 12 - iFontSkip;
                if (oled->cursor_x + iLen > oled->oled_x) // clip right edge
                    iLen = oled->oled_x - oled->cursor_x;
                _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);
                _oled_write_datablock(oled, &ucTemp[6+iFontSkip], iLen, render);
                _oled_set_position(oled, oled->cursor_x, oled->cursor_y+1, render);
                _oled_write_datablock(oled, &ucTemp[18+iFontSkip], iLen, render);
                oled->cursor_x += iLen;
                if (oled->cursor_x >= oled->oled_x-11 && oled->wrap) // word wrap enabled?
                {
                    oled->cursor_x = 0; // start at the beginning of the next line
                    oled->cursor_y += 2;
                    _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);
                }
                iFontSkip = 0;
            } // if characters are visible
            scroll -= 12;
            i++;
        } // while
        return 0;
    } // 12x16

    else if (size == FONT_16x16) // 8x8 stretched to 16x16
    {
        i = 0;
        iFontSkip = scroll & 15; // number of columns to initially skip
        while (oled->cursor_x < oled->oled_x && oled->cursor_y < (oled->oled_y/8)-1 && msg[i] != 0)
        {
            // stretch the 'normal' font instead of using the big font
            if (scroll < 16) // if characters are visible
            {
                int tx, ty;
                c = msg[i] - 32;
                unsigned char uc1, uc2, ucMask, *pDest;
                s = (unsigned char *)&ucFont[(int)c*7];
                ucTemp[0] = 0;
                memcpy_P(&ucTemp[1], s, 7);
                if (invert)
                    _invert_bytes(ucTemp, 8);
                // Stretch the font to double width + double height
                memset(&ucTemp[8], 0, 32); // write 32 new bytes
                for (tx=0; tx<8; tx++)
                {
                    ucMask = 3;
                    pDest = &ucTemp[8+tx*2];
                    uc1 = uc2 = 0;
                    c = ucTemp[tx];
                    for (ty=0; ty<4; ty++)
                    {
                        if (c & (1 << ty)) // a bit is set
                            uc1 |= ucMask;
                        if (c & (1 << (ty + 4)))
                            uc2 |= ucMask;
                        ucMask <<= 2;
                    }
                    pDest[0] = uc1;
                    pDest[1] = uc1; // double width
                    pDest[16] = uc2;
                    pDest[17] = uc2;
                }
                iLen = 16 - iFontSkip;
                if (oled->cursor_x + iLen > oled->oled_x) // clip right edge
                    iLen = oled->oled_x - oled->cursor_x;
                _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);
                _oled_write_datablock(oled, &ucTemp[8+iFontSkip], iLen, render);
                _oled_set_position(oled, oled->cursor_x, oled->cursor_y+1, render);
                _oled_write_datablock(oled, &ucTemp[24+iFontSkip], iLen, render);
                oled->cursor_x += iLen;
                if (oled->cursor_x >= oled->oled_x-15 && oled->wrap) // word wrap enabled?
                {
                    oled->cursor_x = 0; // start at the beginning of the next line
                    oled->cursor_y += 2;
                    _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);
                }
                iFontSkip = 0;
            } // if characters are visible
            scroll -= 16;
            i++;
        } // while
        return 0;
    } // 16x16

    else if (size == FONT_6x8) // 6x8 font
    {
        i = 0;
        iFontSkip = scroll % 6;
        while (oled->cursor_x < oled->oled_x && oled->cursor_y < (oled->oled_y/8) && msg[i] != 0)
        {
            if (scroll < 6) // if characters are visible
            {
                c = msg[i] - 32;
                // we can't directly use the pointer to FLASH memory, so copy to a local buffer
                ucTemp[0] = 0;
                memcpy_P(&ucTemp[1], &ucSmallFont[(int)c*5], 5);
                if (invert) _invert_bytes(ucTemp, 6);
                iLen = 6 - iFontSkip;
                if (oled->cursor_x + iLen > oled->oled_x) // clip right edge
                    iLen = oled->oled_x - oled->cursor_x;
                _oled_write_datablock(oled, &ucTemp[iFontSkip], iLen, render); // write character pattern
                //         oledCachedWrite(ucTemp, 6);
                oled->cursor_x += iLen;
                iFontSkip = 0;
                if (oled->cursor_x >= oled->oled_x-5 && oled->wrap) // word wrap enabled?
                {
                    oled->cursor_x = 0; // start at the beginning of the next line
                    oled->cursor_y++;
                    _oled_set_position(oled, oled->cursor_x, oled->cursor_y, render);
                }
            } // if characters are visible
            scroll -= 6;
            i++;
        }
        //    oledCachedFlush(); // write any remaining data
        return 0;
    } // 6x8

    return -1; // invalid size
}

//
// Invert font data
//
static void _invert_bytes(uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; ++i)
    {
        *data = ~(*data);
        data++;
    }
}


#if 0

//
// Scroll the internal buffer by 1 scanline (up/down)
// width is in pixels, lines is group of 8 rows
//
int oled_scroll_buffer(SSOLED *pOLED,
                       int iStartCol, int iEndCol,
                       int iStartRow, int iEndRow, int bUp)
{
    uint8_t b, *s;
    int col, row;

    if (iStartCol < 0 || (iStartCol > 127) || (iEndCol < 0)
        || (iEndCol > 127) || (iStartCol > iEndCol)) // invalid
        return -1;

    if ((iStartRow < 0) || (iStartRow > 7) || (iEndRow < 0)
        || (iEndRow > 7) || (iStartRow > iEndRow))
        return -1;

    if (bUp)
    {
        for (row=iStartRow; row<=iEndRow; row++)
        {
            s = &pOLED->screen[(row * 128) + iStartCol];
            for (col=iStartCol; col<=iEndCol; col++)
            {
                b = *s;
                b >>= 1; // scroll pixels 'up'
                if (row < iEndRow)
                    b |= (s[128] << 7); // capture pixel of row below, except for last row
                *s++ = b;
            } // for col
        } // for row
    } // up
    else // down
    {
        for (row=iEndRow; row>=iStartRow; row--)
        {
            s = &pOLED->screen[(row * 128)+iStartCol];
            for (col=iStartCol; col<=iEndCol; col++)
            {
                b = *s;
                b <<= 1; // scroll down
                if (row > iStartRow)
                    b |= (s[-128] >> 7); // capture pixel of row above
                *s++ = b;
            } // for col
        } // for row
    }
    return 0;

}


//
// Write a repeating byte to the display
//
void oledRepeatByte(SSOLED *pOLED, uint8_t b, int iLen)
{
int j;
int iWidthMask = pOLED->oled_x -1;
int iWidthShift = (pOLED->oled_x == 128) ? 7:6; // assume 128 or 64 pixels wide
int iSizeMask = ((pOLED->oled_x * pOLED->oled_y)/8) -1;
uint8_t ucTemp[128];

     memset(ucTemp, b, (iLen > 128) ? 128:iLen);
     while (((pOLED->screen_offset & iWidthMask) + iLen) >= pOLED->oled_x) // if it will hit the page end
     {
        j = pOLED->oled_x - (pOLED->screen_offset & iWidthMask); // amount we can write in one shot
        _oled_write_datablock(pOLED, ucTemp, j, 1);
        iLen -= j;
        pOLED->screen_offset = (pOLED->screen_offset + j) & iSizeMask;
        _oled_set_position(pOLED, pOLED->screen_offset & iWidthMask, (pOLED->screen_offset >> iWidthShift), 1);
     } // while it needs some help
  _oled_write_datablock(pOLED, ucTemp, iLen, 1);
  pOLED->screen_offset += iLen;
}
/* oledRepeatByte() */




// byte operands for compressing the data
// the first 2 bits are the type, followed by the counts
#define OP_MASK 0xc0
#define OP_SKIPCOPY 0x00
#define OP_COPYSKIP 0x40
#define OP_REPEATSKIP 0x80
#define OP_REPEAT 0xc0


//
// Play a frame of animation data
// The animation data is assumed to be encoded for a full frame of the display
// Given the pointer to the start of the compressed data,
// it returns the pointer to the start of the next frame
// Frame rate control is up to the calling program to manage
// When it finishes the last frame, it will start again from the beginning
//
uint8_t* oled_play_anim_frame(SSOLED *pOLED, uint8_t *pAnimation, uint8_t *pCurrent, int iLen)
{
uint8_t *s;
int i, j;
unsigned char b, bCode;
int iBufferSize = (pOLED->oled_x * pOLED->oled_y)/8; // size in bytes of the display devce
int iWidthMask, iWidthShift;

  iWidthMask = pOLED->oled_x - 1;
  iWidthShift = (pOLED->oled_x == 128) ? 7:6; // 128 or 64 pixels wide
  if (pCurrent == NULL || pCurrent > pAnimation + iLen)
     return NULL; // invalid starting point

  s = (uint8_t *)pCurrent; // start of animation data
  i = 0;
  _oled_set_position(pOLED, 0,0,1);
  while (i < iBufferSize) // run one frame
  {
    bCode = _pgm_read_byte(s++);
    switch (bCode & OP_MASK) // different compression types
    {
      case OP_SKIPCOPY: // skip/copy
        if (bCode == OP_SKIPCOPY) // big skip
        {
           b = _pgm_read_byte(s++);
           i += b + 1;
           _oled_set_position(pOLED, i & iWidthMask, (i >> iWidthShift), 1);
        }
        else // skip/copy
        {
          if (bCode & 0x38)
          {
            i += ((bCode & 0x38) >> 3); // skip amount
            _oled_set_position(pOLED, i & iWidthMask, (i >> iWidthShift), 1);
          }
          if (bCode & 7)
          {
             _oled_write_flashblock(pOLED, s, bCode & 7);
             s += (bCode & 7);
             i += bCode & 7;
          }
       }
       break;
     case OP_COPYSKIP: // copy/skip
       if (bCode == OP_COPYSKIP) // big copy
       {
         b = _pgm_read_byte(s++);
         j = b + 1;
         _oled_write_flashblock(pOLED, s, j);
         s += j;
         i += j;
       }
       else
       {
         j = ((bCode & 0x38) >> 3);
         if (j)
         {
           _oled_write_flashblock(pOLED, s, j);
           s += j;
           i += j;
         }
         if (bCode & 7)
         {
           i += (bCode & 7); // skip
           _oled_set_position(pOLED, i & iWidthMask, (i >> iWidthShift), 1);
         }
       }
       break;
     case OP_REPEATSKIP: // repeat/skip
       j = (bCode & 0x38) >> 3; // repeat count
       b = _pgm_read_byte(s++);
       oledRepeatByte(pOLED, b, j);
       i += j;
       if (bCode & 7)
       {
         i += (bCode & 7); // skip amount
         _oled_set_position(pOLED, i & iWidthMask, (i >> iWidthShift), 1);
       }
       break;
                  
     case OP_REPEAT:
       j = (bCode & 0x3f) + 1;
       b = _pgm_read_byte(s++);
       oledRepeatByte(pOLED, b, j);
       i += j;
       break;  
    } // switch on code type
  } // while rendering a frame
  if (s >= pAnimation + iLen) // we've hit the end, restart from the beginning
     s = pAnimation;
  return s; // return pointer to start of next frame
}

//
// Write a block of flash memory to the display
//
static void _oled_write_flashblock(SSOLED *oled, uint8_t *s, int len)
{
    int j;
    int iWidthMask = oled->oled_x -1;
    int iSizeMask = ((oled->oled_x * oled->oled_y) / 8) - 1;
    // assume 128 or 64 wide
    int iWidthShift = (oled->oled_x == 128) ? 7 : 6;
    uint8_t ucTemp[128];

           // if it will hit the page end
    while (((oled->screen_offset & iWidthMask) + len) >= oled->oled_x)
    {
        j = oled->oled_x - (oled->screen_offset & iWidthMask);

               // amount we can write in one shot
        memcpy_P(ucTemp, s, j);
        _oled_write_datablock(oled, ucTemp, j, 1);

        s += j;
        len -= j;
        oled->screen_offset = (oled->screen_offset + j) & iSizeMask;

        _oled_set_position(oled,
                           oled->screen_offset & iWidthMask,
                           (oled->screen_offset >> iWidthShift),
                           true);
    }
    // while it needs some help

    memcpy_P(ucTemp, s, len);
    _oled_write_datablock(oled, ucTemp, len, 1);

    oled->screen_offset = (oled->screen_offset + len) & iSizeMask;
}


//
// Draw a sprite of any size in any position
// If it goes beyond the left/right or top/bottom edges
// it's trimmed to show the valid parts
// This function requires a back buffer to be defined
// The priority color (0 or 1) determines which color is painted 
// when a 1 is encountered in the source image. 
//
void oled_draw_sprite(SSOLED *pOLED, uint8_t *pSprite, int cx, int cy, int iPitch, int x, int y, uint8_t iPriority)
{
    int tx, ty, dx, dy, iStartX;
    uint8_t *s, *d, uc, pix, ucSrcMask, ucDstMask;
    
    if (x+cx < 0 || y+cy < 0 || x >= pOLED->oled_x || y >= pOLED->oled_y || pOLED->screen == NULL)
        return; // no backbuffer or out of bounds
    dy = y; // destination y
    if (y < 0) // skip the invisible parts
    {
        cy += y;
        y = -y;
        pSprite += (y * iPitch);
        dy = 0;
    }
    if (y + cy > pOLED->oled_y)
        cy = pOLED->oled_y - y;
    iStartX = 0;
    dx = x;
    if (x < 0)
    {
        cx += x;
        x = -x;
        iStartX = x;
        dx = 0;
    }
    if (x + cx > pOLED->oled_x)
        cx = pOLED->oled_x - x;
    for (ty=0; ty<cy; ty++)
    {
        s = &pSprite[iStartX >> 3];
        d = &pOLED->screen[(dy>>3) * pOLED->oled_x + dx];
        ucSrcMask = 0x80 >> (iStartX & 7);
        pix = *s++;
        ucDstMask = 1 << (dy & 7);
        if (iPriority) // priority color is 1
        {
          for (tx=0; tx<cx; tx++)
          {
            uc = d[0];
            if (pix & ucSrcMask) // set pixel in source, set it in dest
              d[0] = (uc | ucDstMask);
            d++; // next pixel column
            ucSrcMask >>= 1;
            if (ucSrcMask == 0) // read next byte
            {
                ucSrcMask = 0x80;
                pix = *s++;
            }
          } // for tx
        } // priorty color 1
        else
        {
          for (tx=0; tx<cx; tx++)
          {
            uc = d[0];
            if (pix & ucSrcMask) // clr pixel in source, clr it in dest
              d[0] = (uc & ~ucDstMask);
            d++; // next pixel column
            ucSrcMask >>= 1;
            if (ucSrcMask == 0) // read next byte
            {
                ucSrcMask = 0x80;
                pix = *s++;
            }
          } // for tx
        } // priority color 0
        dy++;
        pSprite += iPitch;
    } // for ty
}
/* oledDrawSprite() */

//
// Draw a 16x16 tile in any of 4 rotated positions
// Assumes input image is laid out like "normal" graphics with
// the MSB on the left and 2 bytes per line
// On AVR, the source image is assumed to be in FLASH memory
// The function can draw the tile on byte boundaries, so the x value
// can be from 0 to 112 and y can be from 0 to 6
//
void oled_draw_tile(SSOLED *pOLED, const uint8_t *pTile, int x, int y, int iRotation, int bInvert, int bRender)
{
    uint8_t ucTemp[32]; // prepare LCD data here
    uint8_t i, j, k, iOffset, ucMask, uc, ucPixels;
    uint8_t bFlipX=0, bFlipY=0;
    
    if ((x < 0) || (y < 0) || (y > 6) || (x > 112))
        return; // out of bounds

    if (pTile == NULL)
        return; // bad pointer; really? :(

    if (iRotation == ANGLE_180 || iRotation == ANGLE_270 || iRotation == ANGLE_FLIPX)
        bFlipX = 1;

    if (iRotation == ANGLE_180 || iRotation == ANGLE_270 || iRotation == ANGLE_FLIPY)
        bFlipY = 1;
    
    memset(ucTemp, 0, sizeof(ucTemp)); // we only set white pixels, so start from black

    if (iRotation == ANGLE_0 || iRotation == ANGLE_180
        || iRotation == ANGLE_FLIPX || iRotation == ANGLE_FLIPY)
    {
        for (j=0; j<16; j++) // y
        {
            for (i=0; i<16; i+=8) // x
            {
                ucPixels = _pgm_read_byte((uint8_t*)pTile++);
                ucMask = 0x80; // MSB is the first source pixel
                for (k=0; k<8; k++)
                {
                    if (ucPixels & ucMask) // translate the pixel
                    {
                        if (bFlipY)
                            uc = 0x80 >> (j & 7);
                        else
                            uc = 1 << (j & 7);
                        iOffset = i+k;
                        if (bFlipX) iOffset = 15-iOffset;
                        iOffset += (j & 8)<<1; // top/bottom half of output
                        if (bFlipY)
                            iOffset ^= 16;
                        ucTemp[iOffset] |= uc;
                    }
                    ucMask >>= 1;
                } // for k
            } // for i
        } // for j
    }

    else // rotated 90/270
    {
        for (j=0; j<16; j++) // y
        {
            for (i=0; i<16; i+=8) // x
            {
                ucPixels = _pgm_read_byte((uint8_t*)pTile++);
                ucMask = 0x80; // MSB is the first source pixel
                for (k=0; k<8; k++)
                {
                    if (ucPixels & ucMask) // translate the pixel
                    {
                        if (bFlipY)
                            uc = 0x80 >> k;
                        else
                            uc = 1 << k;
                        iOffset = 15-j;
                        if (bFlipX) iOffset = 15-iOffset;
                        iOffset += i<<1; // top/bottom half of output
                        if (bFlipY)
                            iOffset ^= 16;
                        ucTemp[iOffset] |= uc;
                    }
                    ucMask >>= 1;
                } // for k
            } // for i
        } // for j
    }

    if (bInvert)
        _invert_bytes(ucTemp, 32);

    // Send the data to the display
    _oled_set_position(pOLED, x, y, bRender);
    _oled_write_datablock(pOLED, ucTemp, 16, bRender); // top half
    _oled_set_position(pOLED, x,y+1, bRender);
    _oled_write_datablock(pOLED, &ucTemp[16], 16, bRender); // bottom half
}


// Set (or clear) an individual pixel
// The local copy of the frame buffer is used to avoid
// reading data from the display controller
int oled_set_pixel(SSOLED *pOLED, int x, int y, unsigned char ucColor, int bRender)
{
int i;
unsigned char uc, ucOld;

  i = ((y >> 3) * 128) + x;
  if ((i < 0) || (i > 1023)) // off the screen
    return -1;
  _oled_set_position(pOLED, x, y>>3, bRender);

  if (pOLED->screen)
    uc = ucOld = pOLED->screen[i];
  else if (pOLED->res == OLED_132x64 || pOLED->res == OLED_128x128) // SH1106/SH1107 can read data
  {
    uint8_t ucTemp[3];
     ucTemp[0] = 0x80; // one command
     ucTemp[1] = 0xE0; // read_modify_write
     ucTemp[2] = 0xC0; // one data
     _oled_write(pOLED, ucTemp, 3);

     // read a dummy byte followed by the data byte we want
     i2c_read(pOLED->file, pOLED->addr, ucTemp, 2);
     uc = ucOld = ucTemp[1]; // first byte is garbage 
  }
  else
     uc = ucOld = 0;

  uc &= ~(0x1 << (y & 7));
  if (ucColor)
  {
    uc |= (0x1 << (y & 7));
  }
  if (uc != ucOld) // pixel changed
  {
//    oledSetPosition(x, y>>3);
    if (pOLED->screen)
    {
      _oled_write_datablock(pOLED, &uc, 1, bRender);
      pOLED->screen[i] = uc;
    }
    else if (pOLED->res == OLED_132x64 || pOLED->res == OLED_128x128) // end the read_modify_write operation
    {
      uint8_t ucTemp[4];
      ucTemp[0] = 0xc0; // one data
      ucTemp[1] = uc;   // actual data
      ucTemp[2] = 0x80; // one command
      ucTemp[3] = 0xEE; // end read_modify_write operation
      _oled_write(pOLED, ucTemp, 4);
    }
  }
  return 0;
} /* oledSetPixel() */

//
// Load a 128x64 1-bpp Windows bitmap
// Pass the pointer to the beginning of the BMP file
// First pass version assumes a full screen bitmap
//
int oled_load_bmp(SSOLED *pOLED, uint8_t *pBMP, int bInvert, int bRender)
{
int16_t i16;
int iOffBits, q, y, j; // offset to bitmap data
int iPitch;
uint8_t x, z, b, *s;
uint8_t dst_mask;
uint8_t ucTemp[16]; // process 16 bytes at a time
uint8_t bFlipped = false;

  i16 = _pgm_read_word(pBMP);
  if (i16 != 0x4d42) // must start with 'BM'
     return -1; // not a BMP file
  i16 = _pgm_read_word(pBMP + 18);
  if (i16 != 128) // must be 128 pixels wide
     return -1;
  i16 = _pgm_read_word(pBMP + 22);
  if (i16 != 64 && i16 != -64) // must be 64 pixels tall
     return -1;
  if (i16 == 64) // BMP is flipped vertically (typical)
     bFlipped = true;
  i16 = _pgm_read_word(pBMP + 28);
  if (i16 != 1) // must be 1 bit per pixel
     return -1;
  iOffBits = _pgm_read_word(pBMP + 10);
  iPitch = 16;
  if (bFlipped)
  { 
    iPitch = -16;
    iOffBits += (63 * 16); // start from bottom
  }

// rotate the data and send it to the display
  for (y=0; y<8; y++) // 8 lines of 8 pixels
  {
     _oled_set_position(pOLED, 0, y, bRender);
     for (j=0; j<8; j++) // do 8 sections of 16 columns
     {
         s = &pBMP[iOffBits + (j*2) + (y * iPitch*8)]; // source line
         memset(ucTemp, 0, 16); // start with all black
         for (x=0; x<16; x+=8) // do each block of 16x8 pixels
         {
            dst_mask = 1;
            for (q=0; q<8; q++) // gather 8 rows
            {
               b = _pgm_read_byte(s + (q * iPitch));
               for (z=0; z<8; z++) // gather up the 8 bits of this column
               {
                  if (b & 0x80)
                      ucTemp[x+z] |= dst_mask;
                  b <<= 1;
               } // for z
               dst_mask <<= 1;
            } // for q
            s++; // next source byte
         } // for x
         if (bInvert) _invert_bytes(ucTemp, 16);
         _oled_write_datablock(pOLED, ucTemp, 16, bRender);
     } // for j
  } // for y
  return 0;
}

//
// Render a sprite/rectangle of pixels from a provided buffer to the display.
// The row values refer to byte rows, not pixel rows due to the memory
// layout of OLEDs.
// returns 0 for success, -1 for invalid parameter
//
int oled_draw_gfx(SSOLED *pOLED, uint8_t *pBuffer, int iSrcCol, int iSrcRow, int iDestCol, int iDestRow, int iWidth, int iHeight, int iSrcPitch)
{
    int y;
    
    if ((iSrcCol < 0) || (iSrcCol > 127) || (iSrcRow < 0) || (iSrcRow > 7)
        || (iDestCol < 0) || (iDestCol >= pOLED->oled_x)
        || (iDestRow < 0) || (iDestRow >= (pOLED->oled_y >> 3)) || (iSrcPitch <= 0))
        return -1;
    
    for (y=iSrcRow; y<iSrcRow+iHeight; y++)
    {
        uint8_t *s = &pBuffer[(y * iSrcPitch)+iSrcCol];
        oled_set_position(pOLED, iDestCol, iDestRow, 1);
        _oled_write_datablock(pOLED, s, iWidth, 1);
        pBuffer += iSrcPitch;
        iDestRow++;
    } // for y
    return 0;
} /* oledDrawGFX() */
//
// Dump a screen's worth of data directly to the display
// Try to speed it up by comparing the new bytes with the existing buffer
//
void oled_dump_buffer(SSOLED *pOLED, uint8_t *pBuffer)
{
int x, y;
int iLines, iCols;
uint8_t bNeedPos;
uint8_t *pSrc = pOLED->ucScreen;
    
  if (pBuffer == NULL) // dump the internal buffer if none is given
    pBuffer = pOLED->ucScreen;
  if (pBuffer == NULL)
    return; // no backbuffer and no provided buffer
  
  iLines = pOLED->oled_y >> 3;
  iCols = pOLED->oled_x >> 4;
  for (y=0; y<iLines; y++)
  {
    bNeedPos = 1; // start of a new line means we need to set the position too
    for (x=0; x<iCols; x++) // wiring library has a 32-byte buffer, so send 16 bytes so that the data prefix (0x40) can fit
    {
      if (pOLED->ucScreen == NULL || pBuffer == pSrc || memcmp(pSrc, pBuffer, 16) != 0) // doesn't match, need to send it
      {
        if (bNeedPos) // need to reposition output cursor?
        {
           bNeedPos = 0;
           oled_set_position(pOLED, x*16, y, 1);
        }
        _oled_write_datablock(pOLED, pBuffer, 16, 1);
      }
      else
      {
         bNeedPos = 1; // we're skipping a block, so next time will need to set the new position
      }
      pSrc += 16;
      pBuffer += 16;
    } // for x
    pSrc += (128 - pOLED->oled_x); // for narrow displays, skip to the next line
    pBuffer += (128 - pOLED->oled_x);
  } // for y
} /* oledDumpBuffer() */

void oled_draw_line(SSOLED *pOLED, int x1, int y1, int x2, int y2, int bRender)
{
  int temp;
  int dx = x2 - x1;
  int dy = y2 - y1;
  int error;
  uint8_t *p, *pStart, mask, bOld, bNew;
  int xinc, yinc;
  int y, x;
  
  if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 >= pOLED->oled_x || x2 >= pOLED->oled_x || y1 >= pOLED->oled_y || y2 >= pOLED->oled_y)
     return;

  if(abs(dx) > abs(dy)) {
    // X major case
    if(x2 < x1) {
      dx = -dx;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    }

    y = y1;
    dy = (y2 - y1);
    error = dx >> 1;
    yinc = 1;
    if (dy < 0)
    {
      dy = -dy;
      yinc = -1;
    }
    p = pStart = &pOLED->ucScreen[x1 + ((y >> 3) << 7)]; // point to current spot in back buffer
    mask = 1 << (y & 7); // current bit offset
    for(x=x1; x1 <= x2; x1++) {
      *p++ |= mask; // set pixel and increment x pointer
      error -= dy;
      if (error < 0)
      {
        error += dx;
        if (yinc > 0)
           mask <<= 1;
        else
           mask >>= 1;
        if (mask == 0) // we've moved outside the current row, write the data we changed
        {
           oled_set_position(pOLED, x, y>>3, bRender);
           _oled_write_datablock(pOLED, pStart,  (int)(p-pStart), bRender); // write the row we changed
           x = x1+1; // we've already written the byte at x1
           y1 = y+yinc;
           p += (yinc > 0) ? 128 : -128;
           pStart = p;
           mask = 1 << (y1 & 7);
        }
        y += yinc;
      }
    } // for x1    
   if (p != pStart) // some data needs to be written
   {
     oled_set_position(pOLED, x, y>>3, bRender);
     _oled_write_datablock(pOLED, pStart, (int)(p-pStart), bRender);
   }
  }
  else {
    // Y major case
    if(y1 > y2) {
      dy = -dy;
      temp = x1;
      x1 = x2;
      x2 = temp;
      temp = y1;
      y1 = y2;
      y2 = temp;
    } 

    p = &pOLED->ucScreen[x1 + ((y1 >> 3) * 128)]; // point to current spot in back buffer
    bOld = bNew = p[0]; // current data at that address
    mask = 1 << (y1 & 7); // current bit offset
    dx = (x2 - x1);
    error = dy >> 1;
    xinc = 1;
    if (dx < 0)
    {
      dx = -dx;
      xinc = -1;
    }
    for(x = x1; y1 <= y2; y1++) {
      bNew |= mask; // set the pixel
      error -= dx;
      mask <<= 1; // y1++
      if (mask == 0) // we're done with this byte, write it if necessary
      {
        if (bOld != bNew)
        {
          p[0] = bNew; // save to RAM
          oled_set_position(pOLED, x, y1>>3, bRender);
          _oled_write_datablock(pOLED, &bNew, 1, bRender);
        }
        p += 128; // next line
        bOld = bNew = p[0];
        mask = 1; // start at LSB again
      }
      if (error < 0)
      {
        error += dy;
        if (bOld != bNew) // write the last byte we modified if it changed
        {
          p[0] = bNew; // save to RAM
          oled_set_position(pOLED, x, y1>>3, bRender);
          _oled_write_datablock(pOLED, &bNew, 1, bRender);
        }
        p += xinc;
        x += xinc;
        bOld = bNew = p[0];
      }
    } // for y
    if (bOld != bNew) // write the last byte we modified if it changed
    {
      p[0] = bNew; // save to RAM
      oled_set_position(pOLED, x, y2>>3, bRender);
      _oled_write_datablock(pOLED, &bNew, 1, bRender);
    }
  } // y major case
} /* oledDrawLine() */

//
// Draw a string with a fractional scale in both dimensions
// the scale is a 16-bit integer with and 8-bit fraction and 8-bit mantissa
// To draw at 1x scale, set the scale factor to 256. To draw at 2x, use 512
// The output must be drawn into a memory buffer, not directly to the display
//
int oled_string_scaled(SSOLED *pOLED, int x, int y, char *szMsg, int iSize, int bInvert, int iXScale, int iYScale, int iRotation)
{
uint32_t row, col, dx, dy;
uint32_t sx, sy;
uint8_t c, uc, color, *d;
const uint8_t *s;
uint8_t ucTemp[16];
int tx, ty, bit, iFontOff;
int iPitch;
//int iOffset;
int iFontWidth;

   if (iXScale == 0 || iYScale == 0 || szMsg == NULL || pOLED == NULL || pOLED->ucScreen == NULL || x < 0 || y < 0 || x >= pOLED->oled_x-1 || y >= pOLED->oled_y-1)
      return -1; // invalid display structure
   if (iSize != FONT_8x8 && iSize != FONT_6x8)
      return -1; // only on the small fonts (for now)
   iFontWidth = (iSize == FONT_6x8) ? 6:8;
   s = (iSize == FONT_6x8) ? ucSmallFont : ucFont;
   iPitch = pOLED->oled_x;
   dx = (iFontWidth * iXScale) >> 8; // width of each character
   dy = (8 * iYScale) >> 8; // height of each character
   sx = 65536 / iXScale; // turn the scale into an accumulator value
   sy = 65536 / iYScale;
   while (*szMsg) {
      c = *szMsg++; // debug - start with normal font
      iFontOff = (int)(c-32) * (iFontWidth-1);
      // we can't directly use the pointer to FLASH memory, so copy to a local buffer
      ucTemp[0] = 0; // first column is blank
      memcpy_P(&ucTemp[1], &s[iFontOff], iFontWidth-1);
      if (bInvert) _invert_bytes(ucTemp, iFontWidth);
      col = 0;
      for (tx=0; tx<(int)dx; tx++) {
         row = 0;
         uc = ucTemp[col >> 8];
         for (ty=0; ty<(int)dy; ty++) {
            int nx = 0;
            int ny = 0;
            bit = row >> 8;
            color = (uc & (1 << bit)); // set or clear the pixel
            switch (iRotation) {
               case ROT_0:
                  nx = x + tx;
                  ny = y + ty;
                  break;
               case ROT_90:
                  nx = x - ty;
                  ny = y + tx;
                  break;
               case ROT_180:
                  nx = x - tx;
                  ny = y - ty;
                  break;
               case ROT_270:
                  nx = x + ty;
                  ny = y - tx;
                  break;
            } // switch on rotation direction
            // plot the pixel if it's within the image boundaries
            if (nx >= 0 && ny >= 0 && nx < pOLED->oled_x && ny < pOLED->oled_y) {
               d = &pOLED->ucScreen[(ny >> 3) * iPitch + nx];
               if (color)
                  d[0] |= (1 << (ny & 7));
               else
                  d[0] &= ~(1 << (ny & 7));
            }
            row += sy; // add fractional increment to source row of character
         } // for ty
         col += sx; // add fractional increment to source column
      } // for tx
      // update the 'cursor' position
      switch (iRotation) {
         case ROT_0:
            x += dx;
            break;
         case ROT_90:
            y += dx;
            break;
         case ROT_180:
            x -= dx;
            break;
         case ROT_270:
            y -= dx;
            break;
      } // switch on rotation
   } // while (*szMsg)
   return 0;
} /* oledScaledString() */

//
// For drawing ellipses, a circle is drawn and the x and y pixels are scaled by a 16-bit integer fraction
// This function draws a single pixel and scales its position based on the x/y fraction of the ellipse
//
static void DrawScaledPixel(SSOLED *pOLED, int iCX, int iCY, int x, int y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor)
{
    uint8_t *d, ucMask;
    
    if (iXFrac != 0x10000) x = ((x * iXFrac) >> 16);
    if (iYFrac != 0x10000) y = ((y * iYFrac) >> 16);
    x += iCX; y += iCY;
    if (x < 0 || x >= pOLED->oled_x || y < 0 || y >= pOLED->oled_y)
        return; // off the screen
    d = &pOLED->ucScreen[((y >> 3)*128) + x];
    ucMask = 1 << (y & 7);
    if (ucColor)
        *d |= ucMask;
    else
        *d &= ~ucMask;
} /* DrawScaledPixel() */
//
// For drawing filled ellipses
//
static void DrawScaledLine(SSOLED *pOLED, int iCX, int iCY, int x, int y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor)
{
    int iLen, x2;
    uint8_t *d, ucMask;
    if (iXFrac != 0x10000) x = ((x * iXFrac) >> 16);
    if (iYFrac != 0x10000) y = ((y * iYFrac) >> 16);
    iLen = x*2;
    x = iCX - x; y += iCY;
    x2 = x + iLen;
    if (y < 0 || y >= pOLED->oled_y)
        return; // completely off the screen
    if (x < 0) x = 0;
    if (x2 >= pOLED->oled_x) x2 = pOLED->oled_x-1;
    iLen = x2 - x + 1; // new length
    d = &pOLED->ucScreen[((y >> 3)*128) + x];
    ucMask = 1 << (y & 7);
    if (ucColor) // white
    {
        for (; iLen > 0; iLen--)
            *d++ |= ucMask;
    }
    else // black
    {
        for (; iLen > 0; iLen--)
            *d++ &= ~ucMask;
    }
}

//
// Draw the 8 pixels around the Bresenham circle
// (scaled to make an ellipse)
//
static void BresenhamCircle(SSOLED *pOLED, int iCX, int iCY, int x, int y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor, uint8_t bFill)
{
    if (bFill) // draw a filled ellipse
    {
        // for a filled ellipse, draw 4 lines instead of 8 pixels
        DrawScaledLine(pOLED, iCX, iCY, x, y, iXFrac, iYFrac, ucColor);
        DrawScaledLine(pOLED, iCX, iCY, x, -y, iXFrac, iYFrac, ucColor);
        DrawScaledLine(pOLED, iCX, iCY, y, x, iXFrac, iYFrac, ucColor);
        DrawScaledLine(pOLED, iCX, iCY, y, -x, iXFrac, iYFrac, ucColor);
    }
    else // draw 8 pixels around the edges
    {
        DrawScaledPixel(pOLED, iCX, iCY, x, y, iXFrac, iYFrac, ucColor);
        DrawScaledPixel(pOLED, iCX, iCY, -x, y, iXFrac, iYFrac, ucColor);
        DrawScaledPixel(pOLED, iCX, iCY, x, -y, iXFrac, iYFrac, ucColor);
        DrawScaledPixel(pOLED, iCX, iCY, -x, -y, iXFrac, iYFrac, ucColor);
        DrawScaledPixel(pOLED, iCX, iCY, y, x, iXFrac, iYFrac, ucColor);
        DrawScaledPixel(pOLED, iCX, iCY, -y, x, iXFrac, iYFrac, ucColor);
        DrawScaledPixel(pOLED, iCX, iCY, y, -x, iXFrac, iYFrac, ucColor);
        DrawScaledPixel(pOLED, iCX, iCY, -y, -x, iXFrac, iYFrac, ucColor);
    }
} /* BresenhamCircle() */

//
// Draw an outline or filled ellipse
//
void oled_ellipse(SSOLED *pOLED, int iCenterX, int iCenterY, int32_t iRadiusX, int32_t iRadiusY, uint8_t ucColor, uint8_t bFilled)
{
    int32_t iXFrac, iYFrac;
    int iRadius, iDelta, x, y;
    
    if (pOLED == NULL || pOLED->ucScreen == NULL)
        return; // must have back buffer defined
    if (iRadiusX <= 0 || iRadiusY <= 0) return; // invalid radii
    
    if (iRadiusX > iRadiusY) // use X as the primary radius
    {
        iRadius = iRadiusX;
        iXFrac = 65536;
        iYFrac = (iRadiusY * 65536) / iRadiusX;
    }
    else
    {
        iRadius = iRadiusY;
        iXFrac = (iRadiusX * 65536) / iRadiusY;
        iYFrac = 65536;
    }
    iDelta = 3 - (2 * iRadius);
    x = 0; y = iRadius;
    while (x <= y)
    {
        BresenhamCircle(pOLED, iCenterX, iCenterY, x, y, iXFrac, iYFrac, ucColor, bFilled);
        x++;
        if (iDelta < 0)
        {
            iDelta += (4*x) + 6;
        }
        else
        {
            iDelta += 4 * (x-y) + 10;
            y--;
        }
    }
} /* oledEllipse() */
//
// Draw an outline or filled rectangle
//
void oled_rectangle(SSOLED *pOLED, int x1, int y1, int x2, int y2, uint8_t ucColor, uint8_t bFilled)
{
    uint8_t *d, ucMask, ucMask2;
    int tmp, iOff;
    if (pOLED == NULL || pOLED->ucScreen == NULL)
        return; // only works with a back buffer
    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 ||
       x1 >= pOLED->oled_x || y1 >= pOLED->oled_y || x2 >= pOLED->oled_x || y2 >= pOLED->oled_y) return; // invalid coordinates
    // Make sure that X1/Y1 is above and to the left of X2/Y2
    // swap coordinates as needed to make this true
    if (x2 < x1)
    {
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y2 < y1)
    {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    if (bFilled)
    {
        int x, y, iMiddle;
        iMiddle = (y2 >> 3) - (y1 >> 3);
        ucMask = 0xff << (y1 & 7);
        if (iMiddle == 0) // top and bottom lines are in the same row
            ucMask &= (0xff >> (7-(y2 & 7)));
        d = &pOLED->ucScreen[(y1 >> 3)*128 + x1];
        // Draw top
        for (x = x1; x <= x2; x++)
        {
            if (ucColor)
                *d |= ucMask;
            else
                *d &= ~ucMask;
            d++;
        }
        if (iMiddle > 1) // need to draw middle part
        {
            ucMask = (ucColor) ? 0xff : 0x00;
            for (y=1; y<iMiddle; y++)
            {
                d = &pOLED->ucScreen[(y1 >> 3)*128 + x1 + (y*128)];
                for (x = x1; x <= x2; x++)
                    *d++ = ucMask;
            }
        }
        if (iMiddle >= 1) // need to draw bottom part
        {
            ucMask = 0xff >> (7-(y2 & 7));
            d = &pOLED->ucScreen[(y2 >> 3)*128 + x1];
            for (x = x1; x <= x2; x++)
            {
                if (ucColor)
                    *d++ |= ucMask;
                else
                    *d++ &= ~ucMask;
            }
        }
    }
    else // outline
    {
      // see if top and bottom lines are within the same byte rows
        d = &pOLED->ucScreen[(y1 >> 3)*128 + x1];
        if ((y1 >> 3) == (y2 >> 3))
        {
            ucMask2 = 0xff << (y1 & 7);  // L/R end masks
            ucMask = 1 << (y1 & 7);
            ucMask |= 1 << (y2 & 7);
            ucMask2 &= (0xff >> (7-(y2  & 7)));
            if (ucColor)
            {
                *d++ |= ucMask2; // start
                x1++;
                for (; x1 < x2; x1++)
                    *d++ |= ucMask;
                if (x1 <= x2)
                    *d++ |= ucMask2; // right edge
            }
            else
            {
                *d++ &= ~ucMask2;
                x1++;
                for (; x1 < x2; x1++)
                    *d++ &= ~ucMask;
                if (x1 <= x2)
                    *d++ &= ~ucMask2; // right edge
            }
        }
        else
        {
            int y;
            // L/R sides
            iOff = (x2 - x1);
            ucMask = 1 << (y1 & 7);
            for (y=y1; y <= y2; y++)
            {
                if (ucColor) {
                    *d |= ucMask;
                    d[iOff] |= ucMask;
                } else {
                    *d &= ~ucMask;
                    d[iOff] &= ~ucMask;
                }
                ucMask <<= 1;
                if  (ucMask == 0) {
                    ucMask = 1;
                    d += 128;
                }
            }
            // T/B sides
            ucMask = 1 << (y1 & 7);
            ucMask2 = 1 << (y2 & 7);
            x1++;
            d = &pOLED->ucScreen[(y1 >> 3)*128 + x1];
            iOff = (y2 >> 3) - (y1 >> 3);
            iOff *= 128;
            for (; x1 < x2; x1++)
            {
                if (ucColor) {
                    *d |= ucMask;
                    d[iOff] |= ucMask2;
                } else {
                    *d &= ~ucMask;
                    d[iOff] &= ~ucMask2;
                }
                d++;
            }
        }
    } // outline
}

#endif

