#include "condvar-stl.hh"

#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace stl_condvar {

void StlCondvar::change_parity_or_block(const Parity new_parity) {
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
      if (!lock.try_lock()) {
        continue;
      }
      if (CYCLES < ctr_) {
        return;
      }
      if (Parity::even == new_parity) {
        // Wait for odd ctr_.
        while (is_even()) {
          if (CYCLES < ctr_) {
            return;
          }
          cond_.wait(lock);
          // Yield if awoken spuriously or when signalled by waiter of same
          // parity.
          if (is_even()) {
            continue;
          }
        }
        ctr_++;
      } else {
        // Wait for even ctr_.
        while (is_odd()) {
          if (CYCLES < ctr_) {
            return;
          }
          cond_.wait(lock);
          if (is_odd()) {
            continue;
          }
        }
        ctr_++;
      }
    }
    // Will quickly hang if notify_one() is used instead when two threads of the
    // same parity signal each other and both block.
    cond_.notify_all();
  }
}

void StlCondvar::change_parity_or_timeout(const Parity new_parity) {
  while (true) {
    bool should_signal = false;
    {
      std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
      if (!lock.try_lock()) {
        continue;
      }
      if (CYCLES < ctr_) {
        return;
      }
      // Initialize to no_timeout since the condition may be satisfied when the
      // thread is entered.
      bool succeeded = true;
      if (Parity::even == new_parity) {
        while (is_even()) {
          if (CYCLES < ctr_) {
            return;
          }
          // An inline function (is_odd()) apparently cannot serve as the
          // predicate.
          succeeded = cond_.wait_for(lock, TIMEOUT,
                                     ([&]() { return (1 == (ctr_ % 2)); }));
        }
      } else {
        while (is_odd()) {
          if (CYCLES < ctr_) {
            return;
          }
          succeeded = cond_.wait_for(lock, TIMEOUT,
                                     ([&]() { return (0 == (ctr_ % 2)); }));
        }
      }
      // should_signal is needed because we need to check succeeded inside the
      // lock, but we want to signal outside it.
      if (succeeded) {
        ctr_++;
        should_signal = true;
      }
    }
    // Because the code makes progress even if the wrong waiter is awoken,
    // signalling one works fine.   However, notify_all() is much faster,
    // perhaps by as much as a factor of 2.
    if (should_signal) {
      cond_.notify_all();
    }
  }
}

} // namespace stl_condvar
