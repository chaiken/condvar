// Adapted from listing 7 of
// https://accu.org/journals/overload/32/182/teodorescu/

//	$ sudo strace -vf ./atomic-stack_lib_test-clang 2>&1 | grep futex
//	futex(0x7f070906273c, FUTEX_WAKE_PRIVATE, 2147483647) = 0
//	futex(0x7ffc59918fe8, FUTEX_WAKE_PRIVATE, 1) = 1
//
//	$ sudo ltrace -C -S -f ./atomic-stack_lib_test-gcc-no_sanitize 2>&1 | grep pthread
//	[pid 254040] pthread_mutex_init(0x55b3892163c0, nil) = 0
//	[pid 254040] pthread_mutex_init(0x55b376b01d28, nil) = 0
//	[pid 254040] pthread_mutex_init(0x55b389216508, nil) = 0
//	[pid 254040] pthread_key_create(0, 0x55b376a717b5) = 0
//	[pid 254040] pthread_mutex_init(0x55b389216628, nil) = 0
//	[pid 254040] pthread_key_create(1, 0x55b376a717b5) = 0
//	[pid 254040] pthread_mutex_init(0x55b3892168a8, nil) = 0
//	[pid 254040] pthread_mutex_init(0x55b389216b00, nil) = 0
//	[pid 254040] pthread_key_create(2, 0x55b376a717b5) = 0
//	[pid 254040] pthread_getspecific(2)              = nil
//	[pid 254040] pthread_key_delete(2)               = 0
//	[pid 254040] pthread_mutex_destroy(0x55b3892163c0) = 0
//	[pid 254040] pthread_mutex_destroy(0x55b389216b00) = 0
//	[pid 254040] pthread_mutex_destroy(0x55b3892168a8) = 0
//	[pid 254040] pthread_getspecific(1)              = nil
//	[pid 254040] pthread_key_delete(1)               = 0
//	[pid 254040] pthread_mutex_destroy(0x55b389216628) = 0
//	[pid 254040] pthread_getspecific(0)              = nil
//	[pid 254040] pthread_key_delete(0)               = 0
//	[pid 254040] pthread_mutex_destroy(0x55b389216508) = 0


#include <atomic>

namespace atomic_stack {

template <typename T> struct node {
  T data;
  //  "The reader might remark that next_ is not an atomic type. It doesn’t need
  //  to be, as we always interact with it through the head_ pointer."
  //
  // next must be a raw pointer since atomic types must be trivially copyable.
  // Obviously a unique_ptr is not copyable at all, while shared_ptr is
  // copyable, but not trivially.
  node *next;
  node(const T &val) : data(val), next(nullptr) {}
};

template <typename T> class node_stack {
  // Cannot make head_ a unique_ptr:
  // clang-format off
  // usr/include/c++/14/atomic:218:21: error: static assertion failed: std::atomic requires a trivially copyable type
  // clang-format on
  std::atomic<node<T> *> head_;

public:
  ~node_stack() {
    if (head_.load(std::memory_order_relaxed)) {
      node<T> *nn = head_.load(std::memory_order_relaxed)->next;
      while (nn) {
        node<T> *nnn = nn->next;
        delete nn;
        head_.load(std::memory_order_relaxed)->next = nnn;
        nn = nnn->next;
      }
      delete head_;
    }
  }
  void push(const T &data) {
    node<T> *new_node = new node<T>(data);
    new_node->next = head_.load(std::memory_order_relaxed);
    while (!head_.compare_exchange_weak(new_node->next, new_node,
                                        std::memory_order_release,
                                        std::memory_order_relaxed))
      ;
  }

  T peek() const { return head_.load(std::memory_order_relaxed)->data; }
};

} // namespace atomic_stack
