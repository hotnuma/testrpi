#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgpio.h>
#include <errno.h>
#include <string.h>

// request a line as input.
static struct gpiod_line_request *chip_request_input(int channel,
                                                     unsigned int offset,
                                                     const char *consumer)
{
    struct gpiod_chip *chip = chip_open(channel);
    if (!chip)
        return NULL;

    struct gpiod_line_request *request = NULL;

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings)
        goto close_chip;

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (!line_cfg)
        goto free_settings;

    int ret = gpiod_line_config_add_line_settings(line_cfg,
                                                  &offset, 1, settings);
    if (ret)
        goto free_line_config;

    struct gpiod_request_config *req_cfg = NULL;
    if (consumer)
    {
        req_cfg = gpiod_request_config_new();
        if (!req_cfg)
            goto free_line_config;

        gpiod_request_config_set_consumer(req_cfg, consumer);
    }

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    gpiod_request_config_free(req_cfg);

free_line_config:
    gpiod_line_config_free(line_cfg);

free_settings:
    gpiod_line_settings_free(settings);

close_chip:
    gpiod_chip_close(chip);

    return request;
}

int main()
{
    int offsets[] = {19, 13, 6, 5};
    int SW_NUM = 1;

    struct gpiod_line_request *request =
        chip_request_input(0, offsets[SW_NUM], "get-line-value");

    if (!request)
        return EXIT_FAILURE;

    while (1)
    {
        sleep(1);

        int value = gpiod_line_request_get_value(request, offsets[SW_NUM]);
        printf("switch %d = %d\n", SW_NUM, value);
    }

    gpiod_line_request_release(request);

    return 0;
}


