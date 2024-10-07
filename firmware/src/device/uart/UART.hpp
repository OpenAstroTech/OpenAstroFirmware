#ifndef UART_HPP
#define UART_HPP

#include <zephyr/device.h>

#include <string>

class UART {
public:
    UART(const struct device* uart_dev);
    ~UART();

    void send(std::string data);
    int receive(uint8_t* data, size_t len);

private:
    const struct device* uart_dev;
};

#endif // UART_HPP