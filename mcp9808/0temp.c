
#if 0
#define I2C_BUS "/dev/i2c-1"

int mcp9808_setup(const char *bus)
{
    int file = open(bus, O_RDWR);

    if (file < 0)
        return (-1);

    // Get I2C device, MCP9808 I2C address is 0x18(24)
    ioctl(file, I2C_SLAVE, 0x18);

    // Select configuration register(0x01)
    // Continuous conversion mode, Power-up default(0x00, 0x00)
    char config[3] = {0};
    config[0] = 0x01;
    config[1] = 0x00;
    config[2] = 0x00;
    write(file, config, 3);

    // Select resolution rgister(0x08)
    // Resolution = +0.0625 / C(0x03)
    config[0] = 0x08;
    config[1] = 0x03;
    write(file, config, 2);

    return file;
}

bool mcp9808_read(int file, float *result)
{
        // Read 2 bytes of data from register(0x05)
        // temp msb, temp lsb
        char reg[1] = {0x05};
        write(file, reg, 1);

    char data[2] = {0};

    if (read(file, data, 2) != 2)
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
#endif

