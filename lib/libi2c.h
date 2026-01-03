#ifndef LIBI2C_H
#define LIBI2C_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

inline int i2c_init(int channel, unsigned char addr)
{
    char filename[32];
    sprintf(filename, "/dev/i2c-%d", channel);

    int fd = open(filename, O_RDWR);
    if (fd < 0)
        return -1;

    if (ioctl(fd, I2C_SLAVE, addr) < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}

#endif // LIBI2C_H

