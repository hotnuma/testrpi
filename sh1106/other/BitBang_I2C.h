//
// Bit Bang I2C library
// Copyright (c) 2018 BitBank Software, Inc.
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
//
#ifndef __BITBANG_I2C__
#define __BITBANG_I2C__

int i2c_init(int channel);
unsigned char i2c_test(int file_i2c, unsigned char addr);
int i2c_write(int file_i2c, unsigned char iAddr, unsigned char *pData, int iLen);
// int I2CReadRegister(int file_i2c, unsigned char iAddr,
//                     unsigned char u8Register, unsigned char *pData, int iLen);
int I2CRead(int file_i2c, unsigned char iAddr, unsigned char *pData, int iLen);

#endif //__BITBANG_I2C__

