#include <lx200.h>

#include <zephyr/sys/ring_buffer.h>

void lx200_parse_command(char *command, int cnt, lx200_command_handler_t *handler)
{
    // Parse the command and call the appropriate function
    // TODO: implement properly
    handler->lx200_initialize();
}