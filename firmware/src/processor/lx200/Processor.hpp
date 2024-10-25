#pragma once

#include <Mount.hpp>

#include "CommandParser.hpp"
#include "UARTController.hpp"

namespace lx200
{
    class Processor
    {
    public:
        Processor(const struct device *uart_dev, Mount &mount)
            : parser(mount), uart(uart_dev, parser), mount(mount)
        {
            // Initialization code if needed
        }

    private:
        CommandParser parser;
        UARTController uart;
        Mount mount;
    };

} // namespace lx200
