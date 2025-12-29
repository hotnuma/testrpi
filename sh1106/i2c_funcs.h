#ifndef I2C_FUNCS_H
#define I2C_FUNCS_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

inline int i2c_init(int channel)
{
    char filename[32];
    sprintf(filename, "/dev/i2c-%d", channel);
    return open(filename, O_RDWR);
}

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

#endif // I2C_FUNCS_H

