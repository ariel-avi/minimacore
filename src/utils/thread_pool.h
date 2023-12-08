
#ifndef MINIMACORE_THREAD_POOL_H
#define MINIMACORE_THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <atomic>
#include <mutex>
#include <concepts>
#include <future>
#include <condition_variable>
#include <type_traits>
#include <cstddef>

namespace minimacore {

using std::queue;
using std::vector;
using std::thread;
using std::function;
using std::atomic_bool;
using std::mutex;
using std::unique_lock;
using std::condition_variable;

class thread_pool {

public:
  thread_pool(thread_pool&&) = delete;
  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(thread_pool&&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;
  explicit thread_pool(size_t n = std::thread::hardware_concurrency());
  ~thread_pool();
  void stop();

  template<typename F, typename ... Args>
  std::future<typename std::invoke_result_t<F, Args...>> enqueue(F&& func, Args&& ... args)
  {
    using return_t = typename std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_t()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...));
    std::future<return_t> res = task->get_future();
    {
      unique_lock<mutex> lock(_queue_mutex);
      _queue.emplace([task]() { (*task)(); });
    }
    _cond_var.notify_one();
    return res;
  }

private:
  queue<function<void()>> _queue{};
  mutex _queue_mutex;
  vector<thread> _threads;
  atomic_bool _stop{false};
  condition_variable _cond_var;
};

thread_pool::thread_pool(size_t n)
{
  for (size_t i = 0; i < n; i++) {
    _threads.emplace_back(
            [this] {
              while (true) {
                function < void() > task;
                {
                  unique_lock<mutex> lock(_queue_mutex);
                  _cond_var.wait(lock, [this] { return _stop || !_queue.empty(); });
                  if (_stop && _queue.empty()) return;
                  task = std::move(_queue.front());
                  _queue.pop();
                }
                task();
              }
            }
    );
  }
}

void thread_pool::stop() {
  _stop = true;
  _cond_var.notify_all();
}

thread_pool::~thread_pool()
{
  stop();
  for (auto& t : _threads) if (t.joinable()) t.join();
}

}
#endif //MINIMACORE_THREAD_POOL_H
