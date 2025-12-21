// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Kent Gibson <warthog618@gmail.com>

// mkdir build
// cbuild -Wno-maybe-uninitialized build/test get_line_value.c libgpiod
// ./build/test

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <gpiod.h>
#include <errno.h>
#include <string.h>

/* Request a line as input. */
static struct gpiod_line_request *request_input_line(const char *chip_path,
						     unsigned int offset,
						     const char *consumer)
{
	struct gpiod_chip *chip = gpiod_chip_open(chip_path);
	if (!chip)
		return NULL;

	struct gpiod_line_settings *settings = gpiod_line_settings_new();
	if (!settings)
		goto close_chip;

	gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);

	struct gpiod_line_config *line_cfg = gpiod_line_config_new();
	if (!line_cfg)
		goto free_settings;

	int ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,
						  settings);
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

	struct gpiod_line_request *request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

	gpiod_request_config_free(req_cfg);

free_line_config:
	gpiod_line_config_free(line_cfg);

free_settings:
	gpiod_line_settings_free(settings);

close_chip:
	gpiod_chip_close(chip);

	return request;
}

static bool print_value(unsigned int offset, enum gpiod_line_value value)
{
	if (value == GPIOD_LINE_VALUE_ACTIVE)
    {
		printf("%d=Active\n", offset);
        return true;
	}
    else if (value == GPIOD_LINE_VALUE_INACTIVE)
    {
		printf("%d=Inactive\n", offset);
        return true;
	}

    fprintf(stderr, "error reading value: %s\n", strerror(errno));
    return false;
}

int main(void)
{
	static const char *const chip_path = "/dev/gpiochip0";
	static const unsigned int line_offset = 13;

	struct gpiod_line_request *request = request_input_line(chip_path, line_offset, "get-line-value");
	if (!request)
    {
		fprintf(stderr, "failed to request line: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

    while (1)
    {
        sleep(1);
        enum gpiod_line_value value = gpiod_line_request_get_value(request, line_offset);
        if (print_value(line_offset, value) == false)
            break;
    }
    
	gpiod_line_request_release(request);

	return 0;
}
