#include "Mount.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Mount);

Mount::Mount() {
    LOG_DBG("creating Mount");
    // Constructor implementation
}

Mount::~Mount() {
    // Destructor implementation
}

void Mount::initialize() {
    LOG_INF("Initializing the mount");
    // Initialization code
}

void Mount::moveTo(double ra, double dec) {
    LOG_INF("Moving to RA: %f, DEC: %f", ra, dec);
    // Code to move the mount to the specified RA and DEC
}

void Mount::stop() {
    LOG_INF("Stopping the mount");
    // Code to stop the mount
}
