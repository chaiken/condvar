#include "atomic-stack.impl.hh"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace atomic_stack {
const std::string mystring("hello, world");

TEST(AtomicStackTest, NodeCtor) {
  using namespace testing;
  node new_node(14);
  EXPECT_THAT(new_node.data, Eq(14));
  const std::string mystring("hello, world");
  node newer_node(mystring);
  EXPECT_THAT(newer_node.data, StrEq(mystring));
}

TEST(AtomicStackTest, Push) {
  using namespace testing;
  node_stack<std::string> nstack;
  nstack.push(mystring);
  EXPECT_THAT(nstack.peek(), StrEq(mystring));
}

} // namespace atomic_stack
