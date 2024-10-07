/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

#include "SerialController.hpp"
#include "Mount.hpp"

#include "device/button/Button.hpp"
#include "device/uart/UART.hpp"

#include <zephyr/drivers/uart.h>

#define SLEEP_TIME_MS 10

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_FIRMWARE_LOG_LEVEL);

#include <MeadeLX200Parser.hpp>

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button_spec = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});

static const struct device *uart_dev = DEVICE_DT_GET(DT_ALIAS(uart2));

void button_pressed()
{
	LOG_INF("Button pressed callback");
}

int main(void)
{
	LOG_INF("Starting OpenAstroFirmware");
	LOG_INF("Board: %s", CONFIG_BOARD);
	LOG_INF("MCU Frequency: %u Hz", sys_clock_hw_cycles_per_sec());

	struct uart_config uart_cfg;
	int ret = uart_config_get(uart_dev, &uart_cfg);
	if (ret)
	{
		LOG_ERR("Failed to get console UART configuration");
		return -1;
	}

	LOG_INF("Console UART Baud Rate: %d", uart_cfg.baudrate);

	if (!device_is_ready(uart_dev))
	{
		LOG_ERR("Console UART device not ready\n");
		return ENODEV;
	}

	ret = uart_config_get(uart_dev, &uart_cfg);
	if (ret)
	{
		LOG_ERR("Failed to get control UART configuration");
		return -1;
	}

	LOG_INF("Control UART Baud Rate: %d", uart_cfg.baudrate);

	MeadeLX200Parser meade;
	meade.processCommand(":I#");
	meade.processCommand(":G#");

	Mount mount;
	mount.stop();

	Button button(&button_spec, button_pressed);

	while (1)
	{
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
