#pragma once

#include <zephyr/zbus/zbus.h>

class Mount {
private:
public:
    Mount();  // Constructor
    ~Mount(); // Destructor

    /**
     * @brief Initialize the mount
     */
    void initialize();

    /**
     * @brief Set the Target Ra object
     * 
     * @param h hours
     * @param m minutes
     * @param s seconds
     */
    void setTargetRa(int h, int m, int s);
};

struct MountControlCommand
{
    size_t type;
    /* data */
};

ZBUS_CHAN_DECLARE(mount_control_chan);
