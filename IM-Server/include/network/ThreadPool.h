#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <spdlog/spdlog.h>

class ThreadPool {
public:
    static ThreadPool &GetInstance() {
        static ThreadPool instance(4);
        return instance;
    }
    void Enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.push(std::move(task));
        }
        condition_.notify_one();
    }

private:
    ThreadPool(size_t num_threads) : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i]() {
                spdlog::info("Worker thread {} start.", i);
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);
                        this->condition_.wait(lock, [this] {
                            return this->stop_ || !this->tasks_.empty();
                        });
                        if (this->stop_ && this->tasks_.empty()) {
                            spdlog::info("Worker thread {} exiting.", i);
                            return;
                        }

                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }
                    if (task) {
                        task();
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread &worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    std::vector<std::thread> workers_;         // 打工队列
    std::queue<std::function<void()>> tasks_;  // 共享任务队列
    std::mutex queue_mutex_;                   // 锁
    std::condition_variable condition_;        // 条件变量
    bool stop_;                                // 停止标志
};
#endif