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

#include <mount/Mount.hpp>
#include <mount/MountWorker.hpp>
#include <device/gpio/GPIO.hpp>

#include <zephyr/drivers/stepper.h>

#include "device/uart/InterruptUART.hpp"
#include "processor/lx200/Lx200Processor.hpp"

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
	int32_t position = 0;
	switch (event)
	{
	case STEPPER_EVENT_STEPS_COMPLETED:
		LOG_INF("Steps completed");
		stepper_get_actual_position(stepper, &position);
		LOG_INF("Stepper position: %d", position);
		k_sem_give(&steps_completed_sem);
		break;
	default:
		break;
	}
}

// K_THREAD_STACK_DEFINE(mount_work_stack, 1024 * 16);

// struct k_work_q mount_work_q;
// struct k_work_queue_config mount_work_q_config = {.name = "mount"};

Mount mount;
MountWorker mount_worker(mount);

K_MSGQ_DEFINE(uart_msgq, sizeof(uint8_t), CONFIG_LX200_MESSAGE_BUFFER_SIZE, 1);

Lx200Processor lx200_processor(&uart_msgq, mount_worker);

K_THREAD_DEFINE(
	lx200,							 // Thread identifier
	CONFIG_LX200_STACK_SIZE,		 // Stack size in bytes
	Lx200Processor::thread_function, // Entry point function for the thread
	&lx200_processor, NULL, NULL,	 // Arguments passed to the thread function (arg1, arg2, arg3)
	10,								 // Thread priority (lower number = higher priority)
	0,								 // Thread options (0 = no special options)
	0								 // Delay before starting the thread (0 = start immediately)
);

int main(void)
{
	LOG_INF("Starting OpenAstroFirmware");
	LOG_INF("Board: %s", CONFIG_BOARD);
	LOG_INF("MCU Frequency: %u Hz", sys_clock_hw_cycles_per_sec());

	// k_work_queue_init(&mount_work_q);

	// k_work_queue_start(
	// 	&mount_work_q,
	// 	mount_work_stack,
	// 	K_THREAD_STACK_SIZEOF(mount_work_stack),
	// 	10,
	// 	&mount_work_q_config);

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

	devices::uart::InterruptUART isr_uart(dt::uart_control_dev, &uart_msgq);
	isr_uart.enable();

	// lx200::Processor processor(dt::uart_control_dev, mount);

	// #if SW0_BUTTON
	// 	oaf::device::gpio::Input button(&dt::sw0_button_spec, button_pressed);
	// #endif

	while (1)
	{
#if defined(CONFIG_ARCH_POSIX)
		k_cpu_idle();
#else
		k_sleep(K_MSEC(1000));
#endif
	}

	return 0;
}
