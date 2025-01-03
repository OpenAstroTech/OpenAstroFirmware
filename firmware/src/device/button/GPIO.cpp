#include <device/gpio/GPIO.hpp>

#include <zephyr/logging/log.h>

#include <functional>

LOG_MODULE_REGISTER(GPIO, CONFIG_FIRMWARE_LOG_LEVEL);

using namespace oaf::device::gpio;

_Device::_Device(const gpio_dt_spec *dev) : dev(dev)
{
    // Check if the provided device is ready
    if (!gpio_is_ready_dt(dev))
    {
        LOG_ERR("Device %s is not ready", dev->port->name);
        return;
    }
}

Input::Input(const gpio_dt_spec *dev, std::function<void()> callback) : _Device(dev), callback(callback)
{
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
    gpio_init_callback(&cb_data.cb, handle_gpio_callback, BIT(dev->pin));
    cb_data.context = this;

    ret = gpio_add_callback(dev->port, &cb_data.cb);
    if (ret != 0)
    {
        LOG_ERR("failed to add callback on %s pin %d. code=%d", dev->port->name, dev->pin, ret);
        return;
    }

    LOG_INF("Set up button at %s pin %d", dev->port->name, dev->pin);
}

Input::~Input()
{
    gpio_remove_callback_dt(dev, &cb_data.cb);
}

void Input::handle_gpio_callback(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins)
{
    // get a pointer to the callback in order to receive the actual GPIO instance.
    // This is needed for C/C++ interop.
    Input *context = CONTAINER_OF(cb, CallbackData, cb)->context;
    LOG_DBG("GPIO %d value changed", context->dev->pin);
    context->callback();
}

Output::Output(const gpio_dt_spec *dev) : _Device(dev)
{
    // Configure the GPIO device as output
    int ret = gpio_pin_configure_dt(dev, GPIO_OUTPUT);
    if (ret != 0)
    {
        LOG_ERR("%d: failed to configure %s pin %d", ret, dev->port->name, dev->pin);
        return;
    }
}

Output::~Output()
{
    // Nothing to do here
}

void Output::set(int value)
{
    gpio_pin_set_dt(dev, value);
}

void Output::toggle()
{
    gpio_pin_toggle_dt(dev);
}
