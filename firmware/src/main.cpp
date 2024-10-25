/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include "HardwareConfiguration.hpp"
#include <Mount.hpp>
#include "processor/lx200/Processor.hpp"

#include <device/button/Button.hpp>

LOG_MODULE_REGISTER(main, CONFIG_FIRMWARE_LOG_LEVEL);

void button_pressed()
{
	LOG_INF("Button pressed callback");
}

int main(void)
{
	LOG_INF("Starting OpenAstroFirmware");
	LOG_INF("Board: %s", CONFIG_BOARD);
	LOG_INF("MCU Frequency: %u Hz", sys_clock_hw_cycles_per_sec());

	if (!device_is_ready(dt::uart_control_dev))
	{
		LOG_ERR("Console UART device not ready\n");
		return ENODEV;
	}

	struct uart_config uart_cfg;
	if (uart_config_get(dt::uart_control_dev, &uart_cfg))
	{
		LOG_ERR("Failed to get control UART configuration");
		// return ENODEV;
	} else {
		LOG_INF("Control UART Baud Rate: %d", uart_cfg.baudrate);
	}

	Mount mount;

	lx200::Processor processor(dt::uart_control_dev, mount);

	// lx200_uart_init(dt::uart_control_dev);
	// lx200_uart_enable(dt::uart_control_dev);

#if SW0_BUTTON
	Button button(&dt::sw0_button_spec, button_pressed);
#endif
	
	while (1)
	{
#if defined(CONFIG_ARCH_POSIX)
	k_cpu_idle();
#else
	k_sleep(K_MSEC(10));
#endif
	}

	return 0;
}
