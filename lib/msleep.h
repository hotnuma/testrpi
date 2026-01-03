#ifndef MSLEEP_H
#define MSLEEP_H

#include <unistd.h>

inline int msleep(uint32_t msec)
{
    return usleep(msec * 1000);
}

#endif // MSLEEP_H

