#include <mount/Mount.hpp>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Mount, CONFIG_MOUNT_LOG_LEVEL);

Mount::Mount() {
    LOG_DBG("creating Mount");
    // Constructor implementation
}

Mount::~Mount() {
    // Destructor implementation
}

void Mount::initialize() {

    LOG_INF("Initializing the mount");
}

bool Mount::setTargetDec(int d, unsigned int m, unsigned int s) {
    LOG_INF("Setting the target DEC to %d*%d'%d\"", d, m, s);
    return true;
}

bool Mount::setTargetRa(unsigned int h, unsigned int m, unsigned int s) {
    LOG_INF("Setting the target RA to %d:%d:%d", h, m, s);
    return true;
}
