#pragma once

#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <condition_variable>
#include <future>
#include <queue>
#include <algorithm>

class ThreadPool {
    typedef std::vector<std::thread>            threads_t;
    typedef std::mutex                          mutex_t;
    typedef std::condition_variable             condition_t;
    typedef std::queue<std::function<void()>>   task_queue_t;

public:
    explicit ThreadPool(size_t numberOfThreads):
    threads{threads_t(numberOfThreads)},
    shutdownRequest{false}
    {
        for (size_t i = 0; i < numberOfThreads; i++) {
            threads.at(i) = std::thread(ThreadWorker(this));
        }
    }
    ~ThreadPool() {
        shutdown();
    }

    template <typename FunctionType, typename ...FunctionParameters>
    auto addTask(FunctionType&& function, FunctionParameters... parameters) -> std::future<decltype(function(parameters...))> {
        auto task_ptr = std::make_shared<std::packaged_task<decltype(function(parameters...))()>>(
            std::bind(std::forward<FunctionType>(function), std::forward<FunctionParameters>(parameters)...));

        auto wrapper_func = [task_ptr]() { (*task_ptr)(); };
        {
            std::lock_guard<std::mutex> lock(mutex);
            task_queue.push(wrapper_func);
            condition.notify_one();
        }

        return task_ptr->get_future();
    }
    auto shutdown() -> void {
        {
            std::lock_guard<mutex_t> lock(mutex);
            shutdownRequest = true;
            condition.notify_all();
        }

        std::ranges::for_each(threads, [&](std::thread& worker) -> void {
            if (worker.joinable()) {
                worker.join();
            }
        });
    }

private:
    class ThreadWorker {
    public:
        explicit ThreadWorker(ThreadPool* threadPool):
        thread_pool{threadPool}
        {}

        void operator()() const
        {
            std::unique_lock<mutex_t> lock(thread_pool->mutex);
            while (!thread_pool->shutdownRequest || !thread_pool->task_queue.empty())
            {
                thread_pool->condition.wait(lock, [this] {
                    return this->thread_pool->shutdownRequest || !this->thread_pool->task_queue.empty();
                });

                if (!this->thread_pool->task_queue.empty())
                {
                    auto func = thread_pool->task_queue.front();
                    thread_pool->task_queue.pop();

                    lock.unlock();
                    func();
                    lock.lock();
                }
            }
        }

      private:
        ThreadPool* thread_pool;
    };

    task_queue_t task_queue;
    threads_t threads;
    condition_t condition;
    mutex_t mutex;

    bool shutdownRequest;
};