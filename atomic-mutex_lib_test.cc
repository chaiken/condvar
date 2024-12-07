#include "atomic-mutex.hh"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace atomic_mutex {

TEST(AtomicMutexTest, BasicTest) {
  using namespace testing;

  std::size_t counter1 = 0;
  std::size_t counter2 = 0;

  // Declaring increment() with reference arguments does not suffice.
  std::thread t1{incrementer, std::ref(counter1), std::ref(counter2)};
  std::thread t2{incrementer, std::ref(counter1), std::ref(counter2)};
  std::thread t3{incrementer, std::ref(counter1), std::ref(counter2)};

  t1.join();
  t2.join();
  t3.join();

  EXPECT_THAT(counter1, Eq(static_cast<std::size_t>(3 * CYCLES)));
  EXPECT_THAT(counter2, Eq(counter1));
  EXPECT_THAT(locked_flag.load(std::memory_order_acquire), Eq(false));
}

} // namespace atomic_mutex
