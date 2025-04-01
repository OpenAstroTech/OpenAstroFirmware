#include "UART.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uart, CONFIG_FIRMWARE_LOG_LEVEL);

namespace devices::uart
{
    void UART::queue_rx_byte(uint8_t *byte)
    {
        if (k_msgq_put(uart_msgq, byte, K_NO_WAIT) < 0)
        {
            LOG_WRN("Failed to put data in the message queue");
        }
    }
}