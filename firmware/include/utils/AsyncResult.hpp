#pragma once

#include <optional> // For std::optional
#include <utility>  // For std::move

#include <zephyr/kernel.h>

/**
 * @brief A template class to manage asynchronous results between Zephyr threads using Zephyr Logging.
 *
 * Encapsulates shared data, a mutex for protection, and a semaphore for signaling
 * completion, providing a type-safe C++ interface. Logs messages using the Zephyr Logging API.
 *
 * @tparam ResultType The type of the result data being transferred.
 */
template <typename ResultType>
class AsyncResult
{
public:
    /**
     * @brief Constructor: Initializes internal mutex and semaphore.
     */
    AsyncResult() : is_set_(false)
    {
        k_mutex_init(&mutex_);
        k_sem_init(&semaphore_, 0, 1);
    }

    // Deleted copy constructor and assignment operator
    AsyncResult(const AsyncResult &) = delete;
    AsyncResult &operator=(const AsyncResult &) = delete;

    /**
     * @brief Destructor.
     */
    ~AsyncResult() = default;

    /**
     * @brief Set the result value. Called by the producer/worker thread.
     *
     * @param result The result value to store (will be moved or copied).
     * @param mutex_timeout Timeout for acquiring the internal mutex.
     * @return True if the result was set successfully, false otherwise.
     */
    bool set(ResultType result, k_timeout_t mutex_timeout = K_FOREVER)
    {
        if (k_mutex_lock(&mutex_, mutex_timeout) != 0)
        {
            return false;
        }

        if (is_set_)
        {
            k_mutex_unlock(&mutex_);
            return false;
        }

        result_data_ = std::move(result);
        is_set_ = true;

        k_mutex_unlock(&mutex_);

        k_sem_give(&semaphore_);
        return true;
    }

    /**
     * @brief Wait for and retrieve the result. Called by the consumer/requester thread.
     *
     * @param sem_timeout Timeout for waiting for the result signal (semaphore).
     * @param mutex_timeout Timeout for acquiring the internal mutex after signal received.
     * @return std::optional<ResultType> containing the result if successful/available within timeout,
     * std::nullopt otherwise.
     */
    std::optional<ResultType> get(k_timeout_t sem_timeout, k_timeout_t mutex_timeout = K_FOREVER)
    {
        int sem_status = k_sem_take(&semaphore_, sem_timeout);
        if (sem_status != 0)
        {
            return std::nullopt;
        }

        if (k_mutex_lock(&mutex_, mutex_timeout) != 0)
        {
            // Should we give back the semaphore? Difficult state.
            return std::nullopt;
        }

        ResultType result_copy = result_data_;

        // Optional: Reset state if desired for reuse
        // is_set_ = false; // Resetting state allows set() again

        k_mutex_unlock(&mutex_);

        return result_copy;
    }

    /**
     * @brief Check if the result is ready without blocking.
     * @return True if the result has been set (semaphore count > 0), false otherwise.
     */
    bool is_ready() const
    {
        return k_sem_count_get(&semaphore_) > 0;
    }

private:
    ResultType result_data_;
    volatile bool is_set_;
    struct k_sem semaphore_;
    struct k_mutex mutex_;
};
