//
// Bit Bang I2C library
// Copyright (c) 2018-2019 BitBank Software, Inc.
// Written by Larry Bank (bitbank@pobox.com)
// Project started 10/12/2018
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

#include "BitBang_I2C.h"

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#define PROGMEM
#define false 0
#define true 1
#define memcpy_P memcpy
#define INPUT 1
#define OUTPUT 2

// Initialize the I2C BitBang library
// Pass the pin numbers used for SDA and SCL
// as well as the clock rate in Hz
//
void I2CInit(BBI2C *pI2C)
{
   if (pI2C == NULL)
        return;

   char filename[32];
   sprintf(filename, "/dev/i2c-%d", pI2C->iBus);
   if ((pI2C->file_i2c = open(filename, O_RDWR)) < 0)
         return;

   return;
}

// Test a specific I2C address to see if a device responds
// returns 0 for no response, 1 for a response
//
unsigned char I2CTest(BBI2C *pI2C, unsigned char addr)
{
    unsigned char response = 0;

    if (ioctl(pI2C->file_i2c, I2C_SLAVE, addr) >= 0)
    {
        // probe this address
        unsigned char ucTemp;
        if (read(pI2C->file_i2c, &ucTemp, 1) >= 0)
        response = 1;
    }
    
    return response;
}

// Write I2C data
// quits if a NACK is received and returns 0
// otherwise returns the number of bytes written
int I2CWrite(BBI2C *pI2C, unsigned char iAddr, unsigned char *pData, int iLen)
{
    int rc = 0;

    if (ioctl(pI2C->file_i2c, I2C_SLAVE, iAddr) >= 0)
    {
        if (write(pI2C->file_i2c, pData, iLen) >= 0)
            rc = 1;
    } 
    
    return rc;
}

// Read N bytes starting at a specific I2C internal register
// returns 1 for success, 0 for error
//
int I2CReadRegister(BBI2C *pI2C, unsigned char iAddr,
                    unsigned char u8Register, unsigned char *pData, int iLen)
{
    int i = 0;
    
    if (ioctl(pI2C->file_i2c, I2C_SLAVE, iAddr) >= 0)
    {
        write(pI2C->file_i2c, &u8Register, 1);
            i = read(pI2C->file_i2c, pData, iLen);
    } 
    
    return (i > 0);
}

//
// Read N bytes
//
int I2CRead(BBI2C *pI2C, unsigned char iAddr, unsigned char *pData, int iLen)
{
    int i = 0;

    if (ioctl(pI2C->file_i2c, I2C_SLAVE, iAddr) >= 0)
    {
        i = read(pI2C->file_i2c, pData, iLen);
    } 
    
    return (i > 0);
}


