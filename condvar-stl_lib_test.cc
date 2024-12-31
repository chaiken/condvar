#include "condvar-stl.hh"

#include <chrono>
#include <exception>
#include <stdexcept>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace stl_condvar {

using namespace std::chrono;

constexpr std::size_t NUM_THREADS = 4;

struct CondvarStlTest : public ::testing::Test {
  std::function<void(const Parity)> parity_changer;

  void LaunchThreads() {
    const time_point<steady_clock> start = steady_clock::now();
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
    const time_point<steady_clock> finish = steady_clock::now();
    std::cout << "Elapsed: "
              << duration_cast<microseconds>(finish - start).count()
              << " microS" << std::endl;
  }
};

TEST_F(CondvarStlTest, BlockingTest) {
  using namespace testing;

  // Must be constructed in the same scope where the lambda is defined.
  StlCondvar condvar;

  parity_changer = [&condvar](const Parity new_parity) {
    condvar.change_parity_or_block(new_parity);
  };
  LaunchThreads();

  EXPECT_THAT(condvar.get_count(), Ge(CYCLES));
  EXPECT_THAT(condvar.get_count(), Le(CYCLES + NUM_THREADS));
  std::cout << "Final ctr value is " << condvar.get_count() << std::endl;
}

TEST_F(CondvarStlTest, TimeoutTest) {
  using namespace testing;

  // Must be constructed in the same scope where the lambda is defined.
  StlCondvar condvar;

  parity_changer = [&condvar](const Parity new_parity) {
    condvar.change_parity_or_timeout(new_parity);
  };
  LaunchThreads();

  EXPECT_THAT(condvar.get_count(), Ge(CYCLES));
  EXPECT_THAT(condvar.get_count(), Le(CYCLES + NUM_THREADS));
  std::cout << "Final ctr value is " << condvar.get_count() << std::endl;
}

} // namespace stl_condvar
