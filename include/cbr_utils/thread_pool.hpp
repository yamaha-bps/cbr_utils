// Copyright (c) 2012 Jakob Progsch, Václav Zeman

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.

//    2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.

//    3. This notice may not be removed or altered from any source
//    distribution.

/** @file */

#ifndef CBR_UTILS__THREAD_POOL_HPP_
#define CBR_UTILS__THREAD_POOL_HPP_

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace cbr {

/**
 * @brief Thread pool.
 * @details Pool with a fixed number of workers that can be used to dispatch work.
 */
class ThreadPool
{
public:
  // Constructors
  ThreadPool()                   = default;
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&)      = delete;
  ThreadPool & operator=(const ThreadPool &) = delete;
  ThreadPool & operator=(ThreadPool &&) = delete;
  ~ThreadPool()
  {
    {
      std::scoped_lock lock(m_mtx);
      m_stop = true;
    }
    m_cv.notify_all();
    for (std::thread & worker : m_workers) { worker.join(); }
  }

  /**
   * @brief Construct a new ThreadPool with a given number of workers.
   *
   * @param n_workers Number of workers in the thread pool.
   */
  explicit ThreadPool(const std::size_t n_workers) : m_stop(false)
  {
    for (std::size_t i = 0; i < n_workers; ++i) {
      m_workers.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock lock(this->m_mtx);
            this->m_cv.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty(); });
            if (this->m_stop && this->m_tasks.empty()) { return; }
            task = std::move(this->m_tasks.front());
            this->m_tasks.pop();
          }

          task();
        }
      });
    }
  }

  /**
   * @brief Enqueue task into the thread pool.
   * @details Task is automatically started as soon as worker becomes available.
   *
   * @tparam F Type of the task.
   * @tparam Args Types of the arguments of the task.
   * @param f Task callable object.
   * @param args Arguments of the task.
   * @return std::future for the task.
   */
  template<class F, class... Args>
  std::future<typename std::result_of<F(Args...)>::type> enqueue(F && f, Args &&... args)
  {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::scoped_lock lock(m_mtx);

      // don't allow enqueueing after stopping the pool
      if (m_stop) { throw std::runtime_error("enqueue on stopped ThreadPool"); }

      m_tasks.emplace([task]() { (*task)(); });
    }
    m_cv.notify_one();
    return res;
  }

private:
  /// @cond
  // need to keep track of threads so we can join them
  std::vector<std::thread> m_workers;
  // the task queue
  std::queue<std::function<void()>> m_tasks;

  // synchronization
  std::mutex m_mtx;
  std::condition_variable m_cv;
  bool m_stop;
  /// @endcond
};

}  // namespace cbr

#endif  // CBR_UTILS__THREAD_POOL_HPP_
