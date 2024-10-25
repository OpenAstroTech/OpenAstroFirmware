#include <device/button/Button.hpp>

#include <zephyr/logging/log.h>

#include <functional>

LOG_MODULE_REGISTER(Button, CONFIG_FIRMWARE_LOG_LEVEL);

Button::Button(const gpio_dt_spec *dev, button_callback_t onPressed) : dev(dev), onPressed(onPressed)
{
    // Check if the provided device is ready
    if (!gpio_is_ready_dt(dev))
    {
        LOG_ERR("Device %s is not ready", dev->port->name);
        return;
    }
    
    // Configure the GPIO device as input
    int ret = gpio_pin_configure_dt(dev, GPIO_INPUT);
    if (ret != 0)
    {
        LOG_ERR("%d: failed to configure %s pin %d", ret, dev->port->name, dev->pin);
        return;
    }

    // enable pin interrupts for asynchronous usage
    ret = gpio_pin_interrupt_configure_dt(dev, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0)
    {
        LOG_ERR("failed to configure interrupt on %s pin %d. code=%d", dev->port->name, dev->pin, ret);
        return;
    }

    // initialize the callback to be called on interrupt.
    gpio_init_callback(&button_cb_data.cb, button_pressed, BIT(dev->pin));
    button_cb_data.instance = this;

    ret = gpio_add_callback(dev->port, &button_cb_data.cb);
    if (ret != 0)
    {
        LOG_ERR("failed to add callback on %s pin %d. code=%d", dev->port->name, dev->pin, ret);
        return;
    }

    LOG_INF("Set up button at %s pin %d", dev->port->name, dev->pin);
}

Button::~Button()
{
    gpio_remove_callback(dev->port, &button_cb_data.cb);
}

void Button::button_pressed(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins)
{
    // get a pointer to the callback in order to receive the actual Button instance.
    // This is needed for C/C++ interop.
    Button *instance = CONTAINER_OF(cb, ButtonCallback, cb)->instance;
    LOG_DBG("Button on pin %d pressed", instance->dev->pin);
    instance->onPressed();
}