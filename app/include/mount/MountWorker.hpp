#pragma once

#include <zephyr/kernel.h>

#include "Mount.hpp"
#include "IMount.hpp"

class MountWorker : public IMount
{
    Mount &mount;

    // k_work_q &mount_work_q;
    // k_thread_stack_t *mount_work_stack;

    K_KERNEL_STACK_MEMBER(thread_stack, 1024 * 16);

    struct k_work_q work_q;
    struct k_work_queue_config mount_work_q_config = {.name = "mount"};

public:
    MountWorker(Mount &mount);

    ~MountWorker();

    // IMount interface

    void initialize() override;
    bool setTargetDec(int d, unsigned int m, unsigned int s) override;
    bool setTargetRa(unsigned int h, unsigned int m, unsigned int s) override;

    // template <typename B>
    // static void handler(struct k_work *work);
private:
    auto submit(auto &&fn, auto... args) -> decltype(auto);
};
