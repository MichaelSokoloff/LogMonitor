#pragma once

#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "blocking_queue.hpp"

class threadpool {
private:
    std::vector<std::thread> workers_;
    blocking_queue<std::function<void()>> tasks_;
    bool shutdown_;

public:
    explicit threadpool(size_t num_threads) : shutdown_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {
                        auto maybe_task = tasks_.try_pop();
                        if (!maybe_task.has_value()) {
                            if (tasks_.is_shutdown()) {
                                return;
                            }
                            std::this_thread::yield();
                            continue;
                        }
                        task = std::move(*maybe_task);
                    }

                    task();
                }
                });
        }
    }

    ~threadpool() {
        terminate();
    }

    template<typename F>
    auto enqueue(F&& f) -> std::future<decltype(f())> {
        using return_type = decltype(f());
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
        std::future<return_type> result = task->get_future();

        tasks_.push([task]() { (*task)(); });
        return result;
    }

    void terminate() {
        if (shutdown_) return;
        shutdown_ = true;
        tasks_.shutdown();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};