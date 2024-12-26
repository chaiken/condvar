#include "condvar-pthread.hh"

#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace pthread_condvar {

void PthreadCondvar::change_parity_or_block(const Parity new_parity) {
  // Without the second check of (CYCLES > ctr_) within the loop, the code will
  // not terminate if the last running thread finds that its condition is never
  // satisfied.
  while (CYCLES > ctr_) {
    if (pthread_mutex_lock(&mutex_)) {
      std::cerr << "Failed to lock mutex: " << strerror(errno) << std::endl;
      return;
    }
    if (Parity::even == new_parity) {
      // Must check ctr_ again since the first read is outside  the lock.
      while (is_even() && (CYCLES > ctr_)) {
        pthread_cond_wait(&cond_, &mutex_);
      }
      ctr_++;
    } else {
      while (is_odd() && (CYCLES > ctr_)) {
        pthread_cond_wait(&cond_, &mutex_);
      }
      ctr_++;
    }
    pthread_mutex_unlock(&mutex_);
    // Will quickly hang if pthread_cond_signal() is used instead when two
    // threads of the same parity signal each other and both block.
    pthread_cond_broadcast((&cond_));
  }
}

void PthreadCondvar::change_parity_or_timeout(const Parity new_parity) {
  // Without the second check of (CYCLES > ctr_) within the loop, the code will
  // not terminate if the last running thread finds that its condition is never
  // satisfied.
  while (CYCLES > ctr_) {
    int retval = 0;
    if (pthread_mutex_lock(&mutex_)) {
      std::cerr << "Failed to lock mutex: " << strerror(errno) << std::endl;
      return;
    }
    if (Parity::even == new_parity) {
      // Must check ctr_ again since the first read is outside  the lock.
      while (is_even() && (CYCLES > ctr_)) {
        retval = pthread_cond_timedwait(&cond_, &mutex_, &TIMEOUT);
      }
      // Should the wait have timed out, the precondition for incrementing the
      // counter remains unsatisfied.
      if (!retval)
        ctr_++;
    } else {
      while (is_odd() && (CYCLES > ctr_)) {
        retval = pthread_cond_timedwait(&cond_, &mutex_, &TIMEOUT);
      }
      if (!retval)
        ctr_++;
    }
    pthread_mutex_unlock(&mutex_);
    // Signaling one other waiter is okay because it will time out if it can't
    // proceed.
    if (!retval)
      pthread_cond_signal((&cond_));
  }
}

} // namespace pthread_condvar
