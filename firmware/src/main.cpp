/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>

#include "HardwareConfiguration.hpp"
#include "processor/lx200/Processor.hpp"

#include <Mount.hpp>
#include <device/gpio/GPIO.hpp>

#include <zephyr/drivers/stepper.h>
#include <zephyr/drivers/stepper/stepper_trinamic.h>
#include "../../external/zephyr/include/zephyr/drivers/stepper/stepper_trinamic.h"

LOG_MODULE_REGISTER(main, CONFIG_FIRMWARE_LOG_LEVEL);

const struct device *tmc2209 = DEVICE_DT_GET(DT_NODELABEL(tmc2209));

void button_pressed()
{
	LOG_INF("Button pressed callback");
}

int main(void)
{
	LOG_INF("Starting OpenAstroFirmware");
	LOG_INF("Board: %s", CONFIG_BOARD);
	LOG_INF("MCU Frequency: %u Hz", sys_clock_hw_cycles_per_sec());

	stepper_run(tmc2209, STEPPER_DIRECTION_POSITIVE, 10);

#ifdef CONFIG_USB_DEVICE_STACK
	if (usb_enable(NULL) != 0)
	{
		LOG_ERR("Failed to enable USB");
		return 0;
	}
#endif

	if (!device_is_ready(dt::uart_control_dev))
	{
		LOG_ERR("Console UART device not ready\n");
		return ENODEV;
	}

	struct uart_config uart_cfg;
	if (uart_config_get(dt::uart_control_dev, &uart_cfg))
	{
		LOG_ERR("Failed to get control UART configuration");
	}
	else
	{
		LOG_INF("Control UART Baud Rate: %d", uart_cfg.baudrate);
	}

	Mount mount;

	lx200::Processor processor(dt::uart_control_dev, mount);

#if SW0_BUTTON
	oaf::device::gpio::Input button(&dt::sw0_button_spec, button_pressed);
#endif

	int32_t position = 0;

	while (1)
	{
#if defined(CONFIG_ARCH_POSIX)
		k_cpu_idle();
#else
		// log heartbeat every second to show system is running
		LOG_DBG(".");
		stepper_get_actual_position(tmc2209, &position);
		LOG_INF("Stepper position: %d", position);
		k_sleep(K_MSEC(1000));
#endif
	}

	return 0;
}
