#include <mount/MountWorker.hpp>

#include <memory>
#include <optional>
#include <functional>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <utils/AsyncResult.hpp>

LOG_MODULE_REGISTER(mount_worker);

struct Void
{
};

template <typename B>
struct MountWorkBinding
{
    B binding;

    auto execute()
    {
        if constexpr (std::is_void_v<std::invoke_result_t<B>>)
        {
            binding();
            return Void{};
        }
        else
        {
            return binding();
        }
    }
};

template <typename B>
struct MountWork
{

    k_work work;
    MountWorkBinding<B> *binding;

    using InvokeResult = std::invoke_result_t<B>;

    using Result = std::conditional_t<std::is_void_v<InvokeResult>, Void, InvokeResult>;

    AsyncResult<Result> result;

    static void work_handler(k_work *work)
    {
        // Get the MountWork pointer containing the work item
        auto self = CONTAINER_OF(work, MountWork<B>, work);
        
        // Execute the binding and set the result
        self->result.set(self->binding->execute());
    }

    MountWork(B binding) : binding(new MountWorkBinding<B>({.binding = binding}))
    {
        // Initialize the work item
        k_work_init(&work, work_handler);
    }

    ~MountWork()
    {
        // Free the binding
        delete binding;
    }

    std::optional<Result> submit(k_work_q *work_q)
    {
        // Submit the work to the work queue
        auto submit_res = k_work_submit_to_queue(work_q, &work);
        if (submit_res == 1)
        {
            // Work was already submitted
            return result.get(K_FOREVER);
        }

        LOG_ERR("Failed to submit work. Code: %d", submit_res);

        return std::nullopt;
    }
};

MountWorker::MountWorker(Mount &mount) : mount(mount)
{
    k_work_queue_init(&work_q);

    k_work_queue_start(
        &work_q,
        thread_stack,
        K_THREAD_STACK_SIZEOF(thread_stack),
        10,
        &mount_work_q_config);
}

MountWorker::~MountWorker() = default;

// IMount interface

auto MountWorker::submit(auto &&fn, auto... args) -> decltype(auto)
{
    auto work = std::make_unique<MountWork<decltype(std::bind_front(fn, &mount, args...))>>(std::bind_front(fn, &mount, args...));

    return work->submit(&work_q);
}

void MountWorker::initialize()
{
    submit(&Mount::initialize);
}

bool MountWorker::setTargetDec(int d, unsigned int m, unsigned int s)
{
    return submit(&Mount::setTargetDec, d, m, s).value();
}

bool MountWorker::setTargetRa(unsigned int h, unsigned int m, unsigned int s)
{
    return submit(&Mount::setTargetRa, h, m, s).value();
}