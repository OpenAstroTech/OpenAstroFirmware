#include "InterruptUART.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(uart_isr, CONFIG_FIRMWARE_LOG_LEVEL);

namespace devices::uart
{

    void InterruptUART::enable()
    {
        LOG_INF("Enabling UART interrupt");
        uart_irq_callback_user_data_set(uart_dev, uart_callback, this);
        uart_irq_rx_enable(uart_dev);
    }

    void InterruptUART::disable()
    {
        LOG_INF("Disabling UART interrupt");
        uart_irq_rx_disable(uart_dev);
        uart_irq_tx_disable(uart_dev);
    }

    /**
     * @brief Static callback function for the UART interrupt.
     *        This function is called by the Zephyr kernel when the UART interrupt is triggered.
     *        This function then calls the non-static callback function.
     */
    void InterruptUART::uart_callback(const device *dev, void *controller_ptr)
    {
        if (!uart_irq_update(dev))
        {
            return;
        }

        if (!uart_irq_rx_ready(dev))
        {
            return;
        }
        auto controller = static_cast<InterruptUART *>(controller_ptr);

        // Check if the UART device is ready to process data
        // It has to be called at the beginning of the ISR to update the internal state
        while (uart_irq_update(dev) && uart_irq_is_pending(dev))
        {
            // Check if data is ready to be read (there is data in RX buffer)
            if (uart_irq_rx_ready(dev) == 0)
            {
                // character is not ready to be read. Exit the loop
                continue;
            }

            uint8_t receivedByte;
            int read_result = uart_fifo_read(dev, &receivedByte, 1);

            if (read_result < 0)
            {
                // no character received. Exit the loop
                LOG_WRN("Failed to read from RX. code=%d", read_result);
            }
            else if (read_result == 0)
            {
                // no character received. Exit the loop
                LOG_WRN("No character received.");
            }
            else
            {
                // character received. Queue it for processing
                controller->queue_rx_byte(&receivedByte);
            }
        }
    }
} // namespace devices::uart
