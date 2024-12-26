#include "condvar-pthread.hh"

#include <exception>
#include <stdexcept>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pthread_condvar {

constexpr std::size_t NUM_THREADS = 4;

struct CondvarPthreadTest : public ::testing::Test {
  std::function<void(const Parity)> parity_changer;

  void LaunchThreads() {
    try {
      std::thread t1(parity_changer, Parity::odd);
      std::thread t2(parity_changer, Parity::even);
      std::thread t3(parity_changer, Parity::odd);
      std::thread t4(parity_changer, Parity::even);

      t1.join();
      t2.join();
      t3.join();
      t4.join();
    } catch (const std::system_error &se) {
      std::cerr << "Failed with " << se.what() << std::endl;
    }
  }
};

TEST_F(CondvarPthreadTest, BlockingTest) {
  using namespace testing;

  // Must be constructed in the same scope where the lambda is defined.
  PthreadCondvar condvar;

  parity_changer = [&condvar](const Parity new_parity) {
    condvar.change_parity_or_block(new_parity);
  };

  LaunchThreads();

  EXPECT_THAT(condvar.get_count(), Ge(CYCLES));
  EXPECT_THAT(condvar.get_count(), Lt(CYCLES + NUM_THREADS));
  std::cout << "Final ctr value is " << condvar.get_count() << std::endl;
}

TEST_F(CondvarPthreadTest, TimingoutTest) {
  using namespace testing;

  PthreadCondvar condvar;

  parity_changer = [&condvar](const Parity new_parity) {
    condvar.change_parity_or_timeout(new_parity);
  };

  LaunchThreads();

  EXPECT_THAT(condvar.get_count(), Ge(CYCLES));
  EXPECT_THAT(condvar.get_count(), Lt(CYCLES + NUM_THREADS));
  std::cout << "Final ctr value is " << condvar.get_count() << std::endl;
}

} // namespace pthread_condvar
