#include "UARTController.hpp"

#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_DECLARE(lx200, CONFIG_LX200_LOG_LEVEL);

namespace lx200
{

    UARTController::UARTController(const struct device *uart_dev, CommandParser &parser)
        : uart_dev(uart_dev), parser(parser)
    {
        // Initialize the message queue
        k_msgq_init(&uart_msgq, reinterpret_cast<char *>(uart_msgq_buffer), sizeof(char), LX200_MSGQ_SIZE);

        // Create the UART thread
        uart_thread_id = k_thread_create(
            &uart_thread_data,                                   // k_thread structure
            uart_thread_stack,                                   // stack memory
            K_THREAD_STACK_SIZEOF(uart_thread_stack),            // stack size
            reinterpret_cast<k_thread_entry_t>(thread_function), // entry function
            this,                                                // parameter 1
            NULL,                                                // parameter 2
            NULL,                                                // parameter 3
            LX200_THREAD_PRIORITY,                               // thread priority
            0,                                                   // thread options
            K_NO_WAIT);                                          // delay

        k_thread_name_set(&uart_thread_data, "LX200_UART");

        // Set the callback function for the UART interrupt
        LOG_INF("Setting UART callback");
        int ret = uart_irq_callback_user_data_set(uart_dev, uart_callback, this);
        if (ret)
        {
            LOG_ERR("Failed to set UART callback. code=%d", ret);
        }

        // Enable the RX interrupt
        LOG_INF("Enabling UART RX interrupt");
        uart_irq_rx_enable(uart_dev);
    }

    UARTController::~UARTController()
    {
        // Disable the RX interrupt
        uart_irq_rx_disable(uart_dev);
    }

    /**
     * @brief Static callback function for the UART interrupt. 
     *        This function is called by the Zephyr kernel when the UART interrupt is triggered.
     *        This function then calls the non-static callback function.
     */
    void UARTController::uart_callback(const device *dev, void *controller_ptr)
    {
        auto controller = static_cast<UARTController *>(controller_ptr);

        // Check if the UART device is ready to process data
        // It has to be called at the beginning of the ISR to update the internal state
        while (uart_irq_update(dev) && uart_irq_is_pending(dev))
        {
            // Check if data is ready to be read
            int ret = uart_irq_rx_ready(dev);
            if (ret == 0)
            {
                // character is not ready to be read. Exit the loop
                continue;
            }
            else if (ret < 0)
            {
                LOG_WRN("Failed to check if data is ready to be read");
                continue;
            }

            uint8_t receivedByte;
            ret = uart_fifo_read(dev, &receivedByte, 1);

            if (ret < 0)
            {
                // no character received. Exit the loop
                LOG_WRN("No character received. code=%d", ret);
                continue;
            }

            // Add the received byte to the message queue. This will be processed by the UART thread.
            ret = k_msgq_put(&controller->uart_msgq, &receivedByte, K_NO_WAIT);

            if (ret < 0)
            {
                LOG_WRN("Failed to put data in the message queue. code=%d", ret);
                continue;
            }
        }
    }

    void UARTController::thread_function(void *controller_ptr, void *arg2, void *arg3)
    {
        auto controller = static_cast<UARTController *>(controller_ptr);

        while (true)
        {
            std::string command = "";

            while (true)
            {
                char data = 0;

                // Wait for a character to be received. This is a blocking call on this thread.
                k_msgq_get(&controller->uart_msgq, &data, K_FOREVER);

                if (command.size() == 0 && data != LX200_COMMAND_START)
                {
                    // Command does not start with the start character. Skip the character
                    break;
                }

                // append char to the command
                command += data;

                if (data == LX200_COMMAND_END)
                {
                    // Command end character received. Break the loop
                    break;
                }
            }

            if (command.size() > 0)
            {
                LOG_INF("Command received: %s", command.c_str());

                controller->parser.parse(command);
            }
        }
    }

} // namespace lx200