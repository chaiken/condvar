#include "atomic-flag.hh"

#include <chrono>
#include <exception>
#include <stdexcept>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace atomic_flag {

using namespace std::chrono;

constexpr std::size_t NUM_THREADS = 4;

struct AtomicFlagTest : public ::testing::Test {
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

TEST_F(AtomicFlagTest, BlockingTest) {
  using namespace testing;

  // Must be constructed in the same scope where the lambda is defined.
  AtomicFlag af;

  parity_changer = [&af](const Parity new_parity) {
    af.change_parity_or_block(new_parity);
  };
  LaunchThreads();
  af.initial_unlock();

  EXPECT_THAT(af.get_count(), Ge(CYCLES));
  EXPECT_THAT(af.get_count(), Le(CYCLES + NUM_THREADS));
  std::cout << "Final ctr value is " << af.get_count() << std::endl;
}

} // namespace atomic_flag
