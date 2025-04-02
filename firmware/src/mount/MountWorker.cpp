#include <mount/MountWorker.hpp>

#include <memory>
#include <optional>
#include <functional>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <utils/AsyncResult.hpp>

LOG_MODULE_REGISTER(mount_worker);

/**
 * @brief An empty struct used to indicate a void return type.
 */
struct Void
{
};

/**
 * @brief A binding class that wraps a callable object (function, lambda, etc.) and
 *        provides an execute method to call it.
 *
 * @tparam B The type of the binding (callable object).
 */
template <typename B>
struct MountWorkBinding
{
    B binding;

    /**
     * @brief Executes the binding and returns the result.
     *        In case of a void return type, it returns an empty Void struct.
     */
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
    /**
     * @brief A work item that wraps a binding and executes it in a separate thread.
     *        It is important to define the work item as first in the struct to ensure
     *        proper alignment and size calculations during CONTAINER_OF usage.
     */
    k_work work;

    /**
     * @brief A pointer to the binding that will be executed.
     */
    MountWorkBinding<B> *binding;

    /**
     * @brief The result of the binding execution.
     *        It uses AsyncResult to manage the result asynchronously.
     */
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

    Result submit(k_work_q *work_q)
    {
        // Submit the work to the work queue
        auto submit_res = k_work_submit_to_queue(work_q, &work);

        if (submit_res != 1)
        {
            // Work was not submitted
            LOG_ERR("Failed to submit work. Code: %d", submit_res);

            k_oops();
        }

        auto result_value_opt = result.get(K_FOREVER);

        if (!result_value_opt)
        {
            // Failed to get the result
            LOG_ERR("Failed to get result");
            k_oops();
        }

        return result_value_opt.value();
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
    auto binding = std::bind_front(fn, &mount, args...);
    auto work = std::make_unique<MountWork<decltype(binding)>>(binding);

    // Execute the work and return the result (will be Void for void functions)
    auto result = work->submit(&work_q);

    // For void functions, we don't return anything
    if constexpr (std::is_same_v<decltype(result), Void>)
    {
        return;
    }
    else
    {
        return result;
    }
}

void MountWorker::initialize()
{
    submit(&Mount::initialize);
}

bool MountWorker::setTargetDec(int d, unsigned int m, unsigned int s)
{
    return submit(&Mount::setTargetDec, d, m, s);
}

bool MountWorker::setTargetRa(unsigned int h, unsigned int m, unsigned int s)
{
    return submit(&Mount::setTargetRa, h, m, s);
}