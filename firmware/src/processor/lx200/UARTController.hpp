#pragma once

#include "CommandParser.hpp"

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#define LX200_THREAD_STACK_SIZE 2024
#define LX200_THREAD_PRIORITY 10

#define LX200_MSGQ_SIZE 128

#define LX200_COMMAND_START ':'
#define LX200_COMMAND_END '#'

namespace lx200
{
    class UARTController
    {
    private:
        const struct device *uart_dev;

        CommandParser &parser;

        struct k_msgq uart_msgq;
        char uart_msgq_buffer[LX200_MSGQ_SIZE];

        K_THREAD_STACK_MEMBER(uart_thread_stack, LX200_THREAD_STACK_SIZE);
        struct k_thread uart_thread_data;
        k_tid_t uart_thread_id;

    public:
        UARTController(const struct device *uart_dev, CommandParser &parser);
        ~UARTController();

    private:
        static void uart_callback(const struct device *dev, void *controller_ptr);
        static void thread_function(void *controller_ptr, void *arg2, void *arg3);
    };
};