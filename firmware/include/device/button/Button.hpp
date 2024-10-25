#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <functional>

using button_callback_t = std::function<void()>;

class Button
{
private:
    const gpio_dt_spec *dev;
    button_callback_t onPressed;

    struct ButtonCallback {
        struct gpio_callback cb;
        Button *instance;
    } button_cb_data;
public:
    Button(const gpio_dt_spec *dev, button_callback_t onPressed);
    ~Button();
private:
    static void button_pressed(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins);
};

#endif // BUTTON_HPP