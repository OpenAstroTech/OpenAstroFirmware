#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <functional>

using button_callback_t = std::function<void()>;

namespace oaf
{
    namespace device
    {
        namespace gpio
        {
            class _Device
            {
            protected:
                const gpio_dt_spec *dev;

            public:
                _Device(const gpio_dt_spec *dev);
                ~_Device() = default;
            };

            class Input : public _Device
            {
            private:
                std::function<void()> callback;

                struct CallbackData
                {
                    struct gpio_callback cb;
                    Input *context;
                } cb_data;

            public:
                Input(const gpio_dt_spec *dev, std::function<void()> callback = nullptr);
                ~Input();

            private:
                static void handle_gpio_callback(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins);
            };

            class Output : public _Device
            {

            public:
                Output(const gpio_dt_spec *dev);
                ~Output();

                void set(int value);

                void toggle();
            };
        } // namespace gpio
    } // namespace device
} // namespace oaf