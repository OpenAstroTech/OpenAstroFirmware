#pragma once

#include "UART.hpp"

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart_pipe.h>

namespace devices::uart
{
    class InterruptUART : public UART
    {

        const device *uart_dev;

    public:
        InterruptUART(const device *uart_dev, k_msgq *uart_msgq) : UART(uart_msgq), uart_dev(uart_dev)
        {
            // nothing to do
        }

        ~InterruptUART()
        {
            disable();
        }

        void enable() override;

        void disable() override;

    private:
        static void uart_callback(const struct device *dev, void *self_ptr);
    };
}
