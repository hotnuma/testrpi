#include "mcp9808.h"

bool mcp9808_init(MCP9808 *mcp9808, int channel, uint8_t addr)
{
    mcp9808->addr = addr;
    mcp9808->file = i2c_init(channel, addr);

    if (mcp9808->file < 0)
        return false;

    // select configuration register(0x01)
    // continuous conversion mode, power-up default (0x00, 0x00)
    char config[3] = {0};
    config[0] = 0x01;
    config[1] = 0x00;
    config[2] = 0x00;
    write(mcp9808->file, config, 3);

    // select resolution register (0x08)
    // resolution = +0.0625 / C (0x03)
    config[0] = 0x08;
    config[1] = 0x03;
    write(mcp9808->file, config, 2);

    return true;
}

bool mcp9808_read(MCP9808 *mcp9808, float *result)
{
    if (!mcp9808 || mcp9808->file < 0 || !result)
        return false;

    // read 2 bytes of data from register(0x05)
    // temp msb, temp lsb

    char reg[1] = {0x05};
    write(mcp9808->file, reg, 1);

    char data[2] = {0};

    if (read(mcp9808->file, data, 2) != 2)
    {
        printf("Error : Input/Output error \n");
        return false;
    }

    // Convert the data to 13-bits
    int temp = ((data[0] & 0x1F) * 256 + data[1]);
    if (temp > 4095)
    {
        temp -= 8192;
    }

    if (!result)
        return false;

    float temp_c = temp * 0.0625;
    *result = temp_c;

    return true;
}


