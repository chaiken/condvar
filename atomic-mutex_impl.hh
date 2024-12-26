#include <thread>

namespace atomic_mutex {

template <typename T> void incrementer(T &op1, T &op2) {
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
