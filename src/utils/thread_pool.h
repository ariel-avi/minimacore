
#ifndef MINIMACORE_THREAD_POOL_H
#define MINIMACORE_THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <atomic>
#include <mutex>
#include <concepts>
#include <condition_variable>

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

  explicit thread_pool(size_t n)
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

  template<typename F, typename ... Args>
  void enqueue(F&& func, Args&& ... args)
  {
    unique_lock<mutex> lock(_queue_mutex);
    _queue.emplace([&]() { std::forward<F>(func)(std::forward<Args>(args)...); });
    _cond_var.notify_one();
  }

  ~thread_pool()
  {
    _stop = true;
    _cond_var.notify_all();
    for (auto& t : _threads) t.join();
  }

private:
  queue<function<void()>> _queue{};
  mutex _queue_mutex;
  vector<thread> _threads;
  atomic_bool _stop{false};
  condition_variable _cond_var;
};

}
#endif //MINIMACORE_THREAD_POOL_H
