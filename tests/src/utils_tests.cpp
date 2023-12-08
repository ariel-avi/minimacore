
#include <gtest/gtest.h>
#include <thread_pool.h>
#include <chrono>
#include <iostream>

using namespace minimacore;


void do_work(std::atomic_int& counter)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  counter++;
}

TEST(ThreadPool, RunConcurrently)
{
  static std::atomic_int counter{0};

  thread_pool pool(2);
  for (size_t i = 0; i < 4; i++)
    pool.enqueue(&do_work, counter);

  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  EXPECT_LT(counter, 4);
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  EXPECT_EQ(counter, 4);

  for (size_t i = 0; i < 4; i++)
    pool.enqueue(&do_work, counter);

  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  EXPECT_LT(counter, 8);
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  EXPECT_EQ(counter, 8);
}
