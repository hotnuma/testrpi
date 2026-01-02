#ifndef LIBGPIO_H
#define LIBGPIO_H

#include <gpiod.h>

struct gpiod_chip* chip_open(int channel)
{
    char chip_path[32];
    sprintf(chip_path, "/dev/gpiochip%d", channel);

    return gpiod_chip_open(chip_path);
}

#endif // LIBGPIO_H
