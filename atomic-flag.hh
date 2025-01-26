#include <atomic>
#include <chrono>
#include <iostream>

namespace atomic_flag {

// Lower bound for the number of times the counter will be incremented since
// threads which are waiting when the first thread hits the limit will continue
// to increment the counter.
constexpr std::size_t CYCLES = 1000;
constexpr std::chrono::microseconds TIMEOUT{1};

enum class Parity { even, odd };

class AtomicFlag {
public:
  constexpr std::size_t get_count() const { return ctr_; }

  constexpr bool is_odd() { return (1 == (ctr_ & 1)); }
  constexpr bool is_even() { return (0 == (ctr_ & 1)); }

  // Change the parity of ctr_ to the value new_parity, blocking until the
  // opposite parity is first observed.  Send a broadcast wakeup when complete.
  void change_parity_or_block(const Parity new_parity);

  // For the tests, since the first thread which starts should get the lock.
  void initial_unlock() { flag_.clear(std::memory_order_relaxed); }
  // Change the parity of ctr_ to the value new_parity, blocking until the
  // opposite parity is first observed or a timeout is reached. Send a signal
  // upon success to one other waiter.
  // void change_parity_or_timeout(const Parity new_parity);

private:
  // https://en.cppreference.com/w/cpp/atomic/ATOMIC_FLAG_INIT
  // Since C++20, "default constructor of std::atomic_flag initializes it to
  // clear state."
  std::atomic_flag flag_;
  std::size_t ctr_ = 0;
};

} // namespace atomic_flag
