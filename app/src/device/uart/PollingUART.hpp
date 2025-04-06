#pragma once

#include "UART.hpp"

#include <zephyr/device.h>

namespace devices::uart
{

    /**
     * @class PollingUART
     * @brief Polling-based UART interface.
     *
     * This class provides a polling-based implementation of the UART interface.
     * It allows for sending and receiving data without using interrupts.
     */
    class PollingUART : public UART
    {
    public:
        PollingUART(const device *uart_dev, k_msgq *uart_msgq);

        ~PollingUART();

        void enable() override;

        void disable() override;

        static void thread_function(void *self_ptr, void *arg2, void *arg3);

    private:
        const device *uart_dev;
        bool is_running = false;

        constexpr static size_t thread_stack_size = KB(4);
        K_KERNEL_STACK_MEMBER(thread_stack, KB(4));

        k_thread thread;
    };
}