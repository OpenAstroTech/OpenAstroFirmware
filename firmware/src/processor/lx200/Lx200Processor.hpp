#pragma once

#include <zephyr/kernel.h>

#include "Lx200CommandHandler.hpp"
#include "Lx200Parser.hpp"

#include <mount/MountWorker.hpp>
#include <mount/IMount.hpp>

class Lx200Processor : public Lx200CommandHandler
{
private:
    char buffer[CONFIG_LX200_MESSAGE_BUFFER_SIZE];
    size_t buffer_index = 0;

    Lx200Parser parser;

    k_msgq *msgq;

    MountWorker &mount_worker;

public:
    Lx200Processor(k_msgq *msgq, MountWorker &mount_worker)
        : msgq(msgq), mount_worker(mount_worker) {}

    ~Lx200Processor() = default;

    static void thread_function(void *processor_ptr, void *arg2, void *arg3);

    // Lx200CommandHandler interface
    void handleInitialize() override;

    void handleSetDec(int degrees, unsigned int arcminutes,
                      unsigned int arcseconds) override;

    void handleSetRa(unsigned int hours, unsigned int minutes,
                     unsigned int seconds) override;

protected:
    void process(char data);

private:
    void resetBuffer();
};
