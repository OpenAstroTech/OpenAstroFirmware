#pragma once

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>

namespace devices::uart
{
    /**
     * @class UART
     * @brief Base class for UART (Universal Asynchronous Receiver-Transmitter) interfaces.
     *
     * This abstract class provides a common interface for UART devices across different platforms.
     * It handles the basic functionality of a UART interface including enabling/disabling the interface
     * and managing received data through a message queue.
     *
     * @note This is an abstract class that requires derived classes to implement the enable() and disable() methods.
     *
     * @see enable()
     * @see disable()
     */
    class UART
    {
    private:
        k_msgq *uart_msgq;

    public:
        /**
         * @brief Construct a new UART object.
         *
         * @param uart_dev The device structure for the UART hardware.
         * @param uart_msgq The message queue for received data.
         */
        UART(k_msgq *uart_msgq) : uart_msgq(uart_msgq)
        {
            // nothing to do
        }

        /**
         * @brief Destroy the UART object.
         */
        ~UART() = default;

        /**
         * @brief Enables the UART interface.
         *
         * This function activates the UART hardware, allowing for serial communication to begin.
         * It should be called after initialization and before any attempt to send or receive data.
         */
        virtual void enable() = 0;

        /**
         * @brief Disables the UART interface.
         *
         * This function deactivates the UART hardware, stopping all serial communication.
         * It should be called before deinitialization.
         */
        virtual void disable() = 0;

    protected:
        /**
         * @brief Queues a received byte for processing.
         *
         * This method adds a received byte to the RX queue for later processing.
         * It's typically called from an interrupt service routine when data is available on the UART.
         *
         * @param byte Pointer to the byte received from the UART
         */
        void queue_rx_byte(uint8_t *byte);
    };
} // namespace devices::uart
