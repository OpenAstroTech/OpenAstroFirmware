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

#include "device/uart/InterruptUART.hpp"
#include "processor/lx200/Lx200Processor.hpp"

LOG_MODULE_REGISTER(main, CONFIG_FIRMWARE_LOG_LEVEL);

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
