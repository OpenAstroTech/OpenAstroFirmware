#include "Lx200Processor.hpp"

#include <string>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lx200, CONFIG_LX200_LOG_LEVEL);

void Lx200Processor::process(char data)
{
    // Check if the data is the start of a new command
    if (buffer_index == 0 && data != ':')
    {
        // Command does not start with the start character. Skip the character
        return;
    }

    // Check if the buffer is full
    if (buffer_index >= CONFIG_LX200_MESSAGE_BUFFER_SIZE)
    {
        LOG_WRN("Buffer overflow. Resetting buffer");
        resetBuffer();
    }

    // Add the data to the buffer
    buffer[buffer_index++] = data;

    // Check if the data is the end of a command
    if (data == '#')
    {
        LOG_INF("Command received: %.*s", CONFIG_LX200_MESSAGE_BUFFER_SIZE, buffer);

        // Parse the command
        std::string command(buffer, buffer_index);
        if (!parser.parse(command, this))
        {
            LOG_ERR("Failed to parse command: %s", command.c_str());
        }

        // Reset the buffer for the next command
        resetBuffer();
    }
}

void Lx200Processor::resetBuffer()
{
    // Reset the buffer index and clear the buffer
    buffer_index = 0;
    memset(buffer, 0, CONFIG_LX200_MESSAGE_BUFFER_SIZE);
}

void Lx200Processor::thread_function(void *processor_ptr, void *arg2, void *arg3)
{
    auto processor = static_cast<Lx200Processor *>(processor_ptr);

    while (true)
    {
        char data = 0;

        // Wait for a character to be received. This is a blocking call on this thread.
        k_msgq_get(processor->msgq, &data, K_FOREVER);

        // Process the received data
        processor->process(data);
    }
}

// Lx200CommandHandler interface

void Lx200Processor::handleInitialize()
{
    mount_worker.initialize();
}

void Lx200Processor::handleSetDec(int degrees, unsigned int arcminutes, unsigned int arcseconds)
{
    mount_worker.setTargetDec(degrees, arcminutes, arcseconds);
}

void Lx200Processor::handleSetRa(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
    mount_worker.setTargetRa(hours, minutes, seconds);
}