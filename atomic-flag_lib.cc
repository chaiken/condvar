#include "atomic-flag.hh"

#include <atomic>
#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace atomic_flag {

void AtomicFlag::change_parity_or_block(const Parity new_parity) {
  while (true) {
    while (flag_.test_and_set(std::memory_order_acquire)) {
      flag_.wait(true, std::memory_order_relaxed);
    }
    if (CYCLES < ctr_) {
      flag_.clear(std::memory_order_release);
      return;
    }
    if (Parity::even == new_parity) {
      if (is_odd()) {
        ctr_++;
        flag_.clear(std::memory_order_release);
        // Choosing notify_all() makes the code 3-4x slower despite the fact
        // that the other same-parity thread if awoken will yield and block.
        flag_.notify_one();
      } else {
        // Yield but don't notify if awoken spuriously or when signalled by
        // waiter of same parity.
        flag_.clear(std::memory_order_release);
        continue;
      }
    } else {
      // Wait for even ctr_.
      if (is_even()) {
        ctr_++;
        flag_.clear(std::memory_order_release);
        flag_.notify_one();
      } else {
        flag_.clear(std::memory_order_release);
        continue;
      }
    }
  }
}

} // namespace atomic_flag
