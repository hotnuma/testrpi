// original : https://github.com/ControlEverythingCommunity/MCP9808/blob/master/C/MCP9808.c
// Distributed with a free-will license.
//
// mkdir build
// cbuild build/test main.c && ./build/test
// (gcc -Wall -Wextra -O2 -D_GNU_SOURCE -o build/test main.c)

#include "mcp9808.h"
#include <stdlib.h>

int main()
{
    MCP9808 mcp9808;

    if (!mcp9808_init(&mcp9808, 1, 0x18))
    {
        printf("failed to open the bus...\n");
        return EXIT_FAILURE;
    }

    float temp_c = 0;

    while (1)
    {
        sleep(1);

        mcp9808_read(&mcp9808, &temp_c);

        printf("temp = %.2f °C\n", temp_c);
    }

    return EXIT_SUCCESS;
}

