//
//#include <gtest/gtest.h>
//#include <thread_pool.h>
//#include <chrono>
//#include <iostream>
//
//using namespace minimacore;
//
//class thread_pool_tests : public ::testing::Test {
//protected:
//  std::atomic_int counter{0};
//
//public:
//  void do_work()
//  {
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//    ++counter;
//  }
//};
//
//TEST_F(thread_pool_tests, run_concurrently)
//{
//
//  thread_pool pool(2);
//  for (size_t i = 0; i < 4; i++)
//    auto fut = pool.enqueue(&thread_pool_tests::do_work, this);
//
//  std::this_thread::sleep_for(std::chrono::milliseconds(150));
//  EXPECT_LT(counter, 4);
//  std::this_thread::sleep_for(std::chrono::milliseconds(150));
//  EXPECT_EQ(counter, 4);
//
//  for (size_t i = 0; i < 4; i++)
//    auto fut = pool.enqueue(&thread_pool_tests::do_work, this);
//
//  std::this_thread::sleep_for(std::chrono::milliseconds(150));
//  EXPECT_LT(counter, 8);
//  std::this_thread::sleep_for(std::chrono::milliseconds(150));
//  EXPECT_EQ(counter, 8);
//}
