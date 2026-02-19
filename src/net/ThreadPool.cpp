#include "../include/net/ThreadPool.h"
#include <stdexcept>

ThreadPool::ThreadPool(size_t threadCount)
    : stop(false)
{
    workers.reserve(threadCount);

    for (size_t i = 0; i < threadCount; ++i) {
        workers.emplace_back([this]() {

            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mtx);

                    cv.wait(lock, [this]() {
                        return stop.load() || !tasks.empty();
                    });

                    if (stop.load() && tasks.empty())
                        return;

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                try {
                    task();
                }
                catch (...) {
                    //logger
                }
            }
        });
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (stop.load()) {
            throw std::runtime_error("ThreadPool is stopped. Cannot enqueue new tasks.");
        }

        tasks.push(std::move(task));
    }

    cv.notify_one();
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop.store(true);
    }

    cv.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable())
            worker.join();
    }

    workers.clear();
}

ThreadPool::~ThreadPool() {
    stop = true;
    cv.notify_all();
    for (auto& worker : workers)
        worker.join();
}