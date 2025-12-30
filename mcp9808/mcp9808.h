#ifndef MCP9808_H
#define MCP9808_H

#include <i2c_funcs.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct mcp9808
{
    int file;
    uint8_t addr;

} MCP9808;

bool mcp9808_init(MCP9808 *mcp9808, int channel, int addr);
bool mcp9808_read(MCP9808 *mcp9808, float *result);

#endif // MCP9808_H

