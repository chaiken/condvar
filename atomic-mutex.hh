// Adapted from listings 3 and 5 of
// https://accu.org/journals/overload/32/182/teodorescu/

// $ sudo strace -vf ./atomic-mutex_lib_test-clang-no_sanitize 2>&1 | grep futex
// futex(0x7f1977c6273c, FUTEX_WAKE_PRIVATE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265158] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
 // [pid 265156] futex(0x7f1977800990, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 265157, NULL, FUTEX_BITSET_MATCH_ANY) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265159] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265159] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265159] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265159] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265157] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265159] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// [pid 265158] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0

// [ . . . thousands of similar lines . . . ]

// [pid 265159] futex(0x55f2121afd80, FUTEX_WAKE, 2147483647) = 0
// // [pid 265156] futex(0x7f1976400990, FUTEX_WAIT_BITSET|FUTEX_CLOCK_REALTIME, 265159, NULL, FUTEX_BITSET_MATCH_ANY) = 0
// futex(0x7f1977c62748, FUTEX_WAKE_PRIVATE, 2147483647) = 0

// $ sudo strace -vf ./atomic-mutex_lib_test-clang-no_sanitize 2>&1 | grep robust
// set_robust_list(0x7fa3ddea63a0, 24)     = 0
// [pid 265361] set_robust_list(0x7fa3dda009a0, 24) = 0
// [pid 265362] set_robust_list(0x7fa3dd0009a0, 24) = 0
// [pid 265363] set_robust_list(0x7fa3dc6009a0, 24) = 0


#include <atomic>
#include <functional>
#include <thread>

namespace atomic_mutex {

constexpr uint32_t CYCLES = 1'000'000;

std::atomic<bool> locked_flag = false;

// A lambda must be defined in the translation unit where it is invoked if it
// performs a capture.
void incrementer(std::size_t &op1, std::size_t &op2) {
  for (std::size_t i = 0; i < CYCLES; ++i) {
    // Acquire the lock.
    while (locked_flag.exchange(true, std::memory_order_acquire)) {
      locked_flag.wait(true, std::memory_order_relaxed);
    }
    // Protected operations.
    op1++;
    op2 = op1;
    // Release the lock.
    locked_flag.store(false, std::memory_order_release);
    locked_flag.notify_one();
  }
}

} // namespace atomic_mutex
