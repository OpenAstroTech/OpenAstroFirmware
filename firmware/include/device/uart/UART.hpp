#ifndef UART_HPP
#define UART_HPP

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <string>

class UART {
public:
    UART(const struct device* uart_dev, const uart_callback_t callback);
    ~UART();

    void send(std::string data);
    int receive(uint8_t* data, size_t len);

private:
    const struct device* uart_dev;
    const uart_callback_t callback;
};

#endif // UART_HPP