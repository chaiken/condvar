#include "condvar-pthread.hh"

#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <optional>

namespace pthread_condvar {
namespace {

constexpr int64_t BILLION = 1'000'000'000;

std::optional<timespec> delay_to_expiration_time(const struct timespec &ts) {
  struct timespec now{};
  if (clock_gettime(CLOCK_MONOTONIC, &now)) {
    std::cerr << "clock_gettime() failed with " << strerror(errno) << std::endl;
    return {};
  }
  now.tv_nsec += ts.tv_nsec;
  if (BILLION <= now.tv_nsec) {
    now.tv_sec += (now.tv_nsec - BILLION) / BILLION;
    now.tv_nsec -= BILLION;
  }
  return now;
}

} // namespace

void PthreadCondvar::change_parity_or_block(const Parity new_parity) {
  bool should_return = false;
  do {
    if (pthread_mutex_lock(&mutex_)) {
      std::cerr << "Failed to lock mutex: " << strerror(errno) << std::endl;
      return;
    }
    // Already done.
    if (CYCLES <= ctr_) {
      pthread_mutex_unlock(&mutex_);
      return;
    }
    if (Parity::even == new_parity) {
      // Must check ctr_ again since the first read is outside  the lock.
      while (is_even()) {
        pthread_cond_wait(&cond_, &mutex_);
        // breaking rather than returning causes extra wakeups after exit
        // condition is achieved, but skipping them means that final waiters may
        // never wake.
        if (CYCLES <= ctr_) {
          // Cache the value since ctr_ must be checked inside the lock.
          should_return = true;
          break;
        }
      }
      // Guard against spurious wakeups.
      if (is_odd())
        ctr_++;
    } else {
      while (is_odd()) {
        pthread_cond_wait(&cond_, &mutex_);
        if (CYCLES <= ctr_) {
          should_return = true;
          break;
        }
      }
      if (is_even())
        ctr_++;
    }

    pthread_mutex_unlock(&mutex_);
    // Will quickly hang if pthread_cond_signal() is used instead when two
    // threads of the same parity signal each other and both block.
    pthread_cond_broadcast((&cond_));
    if (should_return)
      return;
  } while (true);
}

void PthreadCondvar::change_parity_or_timeout(const Parity new_parity) {
  do {
    int err = pthread_mutex_trylock(&mutex_);
    if (EBUSY == err) {
      continue;
    } else if (err) {
      std::cerr << "Failed to lock mutex: " << strerror(errno) << std::endl;
      return;
    }

    // Already done.
    if (CYCLES <= ctr_) {
      // Create the complete string to prevent interleaving of output.
      pthread_mutex_unlock(&mutex_);
      return;
    }

    std::optional<struct timespec> expiration =
        delay_to_expiration_time(TIMEOUT);
    if (!expiration.has_value()) {
      std::cerr << "Unable to read clock" << std::endl;
      return;
    }
    int retval = 0;
    if (Parity::even == new_parity) {
      while (is_even()) {
        retval = pthread_cond_timedwait(&cond_, &mutex_, &(expiration.value()));
        if (CYCLES <= ctr_) {
          pthread_mutex_unlock(&mutex_);
          return;
        }
      }
      // Should the wait have timed out, the precondition for incrementing the
      // counter remains unsatisfied.
      if (!retval)
        ctr_++;

      if (CYCLES <= ctr_) {
        pthread_mutex_unlock(&mutex_);
        return;
      }
    } else {
      while (is_odd()) {
        retval = pthread_cond_timedwait(&cond_, &mutex_, &(expiration.value()));
        if (CYCLES <= ctr_) {
          pthread_mutex_unlock(&mutex_);
          return;
        }
      }
      if (!retval)
        ctr_++;

      if (CYCLES <= ctr_) {
        pthread_mutex_unlock(&mutex_);
        return;
      }
    }

    pthread_mutex_unlock(&mutex_);
    // Signalling works fine due to the timeout: the correct thread will
    // eventually awake.
    if (!retval)
      pthread_cond_signal((&cond_));
  } while (true);
}

} // namespace pthread_condvar
