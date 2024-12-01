// Adapted from listing 7 of
// https://accu.org/journals/overload/32/182/teodorescu/

#include <atomic>

namespace atomic_stack {

template <typename T> struct node {
  T data;
  // Could next be a unique_ptr and still work with head_ as atomic type?
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
