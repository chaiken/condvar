#include <pthread.h>

#include <iostream>

namespace pthread_condvar {

// Lower bound for the number of times the counter will be incremented since
// threads which are waiting when the first thread hits the limit will continue
// to increment the counter.
constexpr std::size_t CYCLES = 1000;

// For pthread_cond_timedwait().
const struct timespec TIMEOUT{
    .tv_sec = 0,
    .tv_nsec = 10'000,
};

enum class Parity { even, odd };

// man 3 pthread_cond_init();
//
// pthread_cond_init,   pthread_cond_signal,  pthread_cond_broadcast,  and
// pthread_cond_wait never return an error code.

class PthreadCondvar {
public:
  PthreadCondvar()
      : cond_(PTHREAD_COND_INITIALIZER), mutex_(PTHREAD_MUTEX_INITIALIZER),
        ctr_(0) {}
  //  In the LinuxThreads implementation, no resources are associated with
  //  condition  variables, thus pthread_cond_destroy actually does nothing
  //  except checking that the condition has no waiting threads.
  ~PthreadCondvar() {
    if (pthread_cond_destroy(&cond_)) {
      std::cerr << "Condition variable still has waiters." << std::endl;
    }
    if (pthread_mutex_destroy(&mutex_)) {
      std::cerr << "Locked mutex cannot be destroyed." << std::endl;
    }
  }

  constexpr std::size_t get_count() const { return ctr_; }

  constexpr bool is_odd() { return (1 == (ctr_ & 1)); }
  constexpr bool is_even() { return (0 == (ctr_ & 1)); }

  // Change the parity of ctr_ to the value new_parity, blocking until the
  // opposite parity is first observed.  Send a broadcast wakeup when complete.
  void change_parity_or_block(const Parity new_parity);

  // Change the parity of ctr_ to the value new_parity, blocking until the
  // opposite parity is first observed or a timeout is reached. Send a signal
  // upon success to one other waiter.
  void change_parity_or_timeout(const Parity new_parity);

private:
  pthread_cond_t cond_;
  pthread_mutex_t mutex_;
  std::size_t ctr_;
};

} // namespace pthread_condvar
