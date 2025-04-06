#include "PollingUART.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(uart_poll, CONFIG_FIRMWARE_LOG_LEVEL);

namespace devices::uart
{

    PollingUART::PollingUART(const device *uart_dev, k_msgq *uart_msgq)
        : UART(uart_msgq), uart_dev(uart_dev)
    {
        // nothing to do
        k_thread_create(
            &thread, 
            thread_stack, 
            thread_stack_size,
            thread_function, 
            this, NULL, NULL,
            10,
            0, 
            K_NO_WAIT);

        k_thread_name_set(&thread, "uart_poll");
    }

    PollingUART::~PollingUART()
    {
        disable();
    }

    void PollingUART::enable()
    {
        LOG_INF("Enabling UART polling");
        is_running = true;
    }

    void PollingUART::disable()
    {
    }

    void PollingUART::thread_function(void *self_ptr, void *arg2, void *arg3)
    {
        LOG_INF("Starting UART polling loop");

        auto self = static_cast<PollingUART *>(self_ptr);

        while (true)
        {
            if (self == nullptr || !self->is_running)
            {
                // Wait for the UART to be enabled
                k_sleep(K_MSEC(10));
                continue;
            }

            uint8_t receivedByte;

            if (uart_poll_in(self->uart_dev, &receivedByte) < 0)
            {
                // no character received.
                k_sleep(K_MSEC(10));
                continue;
            }
            else
            {
                // Put the received byte in the message queue
                self->queue_rx_byte(&receivedByte);
            }
        }
    }
}