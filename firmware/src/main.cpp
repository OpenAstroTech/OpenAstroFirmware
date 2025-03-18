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

LOG_MODULE_REGISTER(main, CONFIG_FIRMWARE_LOG_LEVEL);

const struct device *stepper = DEVICE_DT_GET(DT_NODELABEL(stepper0));

K_SEM_DEFINE(steps_completed_sem, 0, 1);

void button_pressed()
{
	LOG_INF("Button pressed callback");
	if (k_sem_take(&steps_completed_sem, K_FOREVER) == 0)
	{
		if (stepper_set_microstep_interval(stepper, 1250000) != 0)
		{
			LOG_ERR("Failed to set microstep interval");
			return;
		}

		if (stepper_move_by(stepper, 1000) != 0)
		{
			LOG_ERR("Failed to move stepper");
			return;
		}
	}
}

void stepper_callback(const struct device *dev, const enum stepper_event event, void *user_data)
{
	switch (event)
	{
	case STEPPER_EVENT_STEPS_COMPLETED:
		LOG_INF("Steps completed");
		k_sem_give(&steps_completed_sem);
		break;
	default:
		break;
	}
}

int main(void)
{
	LOG_INF("Starting OpenAstroFirmware");
	LOG_INF("Board: %s", CONFIG_BOARD);
	LOG_INF("MCU Frequency: %u Hz", sys_clock_hw_cycles_per_sec());

	// stepper_set_microstep_interval(stepper, 500000);

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

	stepper_set_event_callback(stepper, stepper_callback, NULL);
	stepper_set_micro_step_res(stepper, STEPPER_MICRO_STEP_1);
	stepper_enable(stepper, true);
	stepper_set_microstep_interval(stepper, 1250000);
	stepper_move_by(stepper, 1000);

	while (1)
	{
#if defined(CONFIG_ARCH_POSIX)
		k_cpu_idle();
#else
		// log heartbeat every second to show system is running
		LOG_DBG(".");
		stepper_get_actual_position(stepper, &position);

		LOG_INF("Stepper position: %d", position);
		k_sleep(K_MSEC(1000));
#endif
	}

	return 0;
}
