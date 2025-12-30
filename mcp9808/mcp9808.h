#ifndef MCP9808_H
#define MCP9808_H

#include <libi2c.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct mcp9808
{
    int file;
    uint8_t addr;

} MCP9808;

bool mcp9808_init(MCP9808 *mcp9808, int channel, uint8_t addr);
bool mcp9808_read(MCP9808 *mcp9808, float *result);

#endif // MCP9808_H

