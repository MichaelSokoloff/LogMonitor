#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <utility>
#include <limits>

template <typename T>
class blocking_queue {
private:
    std::queue<T> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cv_not_empty_;
    std::condition_variable cv_not_full_;
    size_t max_size_;
    bool shutdown_;

public:
    explicit blocking_queue(size_t max_size = std::numeric_limits<size_t>::max())
        : max_size_(max_size), shutdown_(false) {
    }

    ~blocking_queue() = default;

    blocking_queue(const blocking_queue&) = delete;
    blocking_queue(blocking_queue&&) = delete;
    blocking_queue& operator=(const blocking_queue&) = delete;
    blocking_queue& operator=(blocking_queue&&) = delete;

    void shutdown() noexcept {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            shutdown_ = true;
        }
        cv_not_empty_.notify_all();
        cv_not_full_.notify_all();
    }

    bool is_shutdown() const noexcept {
        std::lock_guard<std::mutex> lock(mtx_);
        return shutdown_;
    }

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_not_full_.wait(lock, [this] {
            return queue_.size() < max_size_ || shutdown_;
            });

        if (shutdown_ && queue_.size() >= max_size_) {
            return;
        }

        queue_.push(item);
        cv_not_empty_.notify_one();
    }

    void push(T&& item) noexcept(std::is_nothrow_move_constructible<T>::value) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_not_full_.wait(lock, [this] {
            return queue_.size() < max_size_ || shutdown_;
            });

        if (shutdown_ && queue_.size() >= max_size_) {
            return;
        }

        queue_.push(std::move(item));
        cv_not_empty_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_not_empty_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
            });

        if (shutdown_ && queue_.empty()) {
            throw std::runtime_error("Queue shutdown: no more items");
        }

        T value = std::move_if_noexcept(queue_.front());
        queue_.pop();
        cv_not_full_.notify_one();
        return value;
    }

    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        if (queue_.empty()) {
            return std::nullopt;
        }

        T value = std::move_if_noexcept(queue_.front());
        queue_.pop();
        cv_not_full_.notify_one();
        return value;
    }

    size_t size() const noexcept {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

    size_t max_size() const noexcept {
        return max_size_;
    }

    void clear() noexcept {
        std::unique_lock<std::mutex> lock(mtx_);
        while (!queue_.empty()) {
            queue_.pop();
        }
        cv_not_full_.notify_all();
    }
};