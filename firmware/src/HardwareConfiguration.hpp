#include <zephyr/device.h>

/**
 * @brief Device tree configuration namespace
 */
namespace dt
{
/**
 * Get button configuration from the devicetree sw0 alias. E.g. this is the user button on a nucleo board.
 */
#if DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
#define SW0_BUTTON 1
    static const struct gpio_dt_spec sw0_button_spec = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
#else
#define SW0_BUTTON 0
#endif /* DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay) */

/**
 * Get UART configuration from the devicetree chosen node. This is the UART used for control commands.
 */
#if DT_NODE_HAS_STATUS(DT_CHOSEN(oaf_uart_control), okay)
    static const struct device *uart_control_dev = DEVICE_DT_GET(DT_CHOSEN(oaf_uart_control));
#endif /* DT_NODE_HAS_STATUS(DT_CHOSEN(sw1), okay) */

}