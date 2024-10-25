#include <device/uart/UART.hpp>

#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(UART, CONFIG_FIRMWARE_LOG_LEVEL);

UART::UART(const struct device* uart_dev, const uart_callback_t callback) : uart_dev(uart_dev), callback(callback) {}

UART::~UART() {}

void UART::send(std::string data) {
    for (char c : data) {
        uart_poll_out(uart_dev, c);
    }
}

int UART::receive(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        int ret = uart_poll_in(uart_dev, &data[i]);
        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}