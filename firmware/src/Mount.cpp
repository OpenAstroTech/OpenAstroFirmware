#include <Mount.hpp>

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
}

void Mount::setTargetDec(int d, int m, int s) {
    LOG_INF("Setting the target DEC to %d*%d'%d\"", d, m, s);
}

void Mount::setTargetRa(int h, int m, int s) {
    LOG_INF("Setting the target RA to %d:%d:%d", h, m, s);
}
