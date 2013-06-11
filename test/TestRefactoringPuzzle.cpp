#include <iostream>
#include "RefactoringPuzzle.h"

#include "gtest/gtest.h"

namespace TestRefactoringPuzzle {
  // Shortcut for testing: do not bother to implement equality on
  // our data structures, but just look at the string representations!

  using namespace RefactoringPuzzle;
  using namespace std;

  template<typename A>
  auto toString(const A &a)
    -> string
  {
    stringstream s;
    s << a;
    return s.str();
  }

  auto add1 = [](int n) { return n+1; };

  TEST(Option, selectAdd1OnNome) {
    auto nothing = none<int>();
    auto result = nothing->select<int>(add1);
    ASSERT_EQ("None", toString(result));
  }
  
  TEST(Option, selectAdd1OnSome) {
    // Some(4)
    auto someFour = some<int>(4);
    
    auto result = someFour->select<int>(add1);
    ASSERT_EQ("Some(5)", toString(result));
  }
  
  TEST(List, runOptionsSucceedsOnAllSome) {
    // Cons(Some(1), Cons(Some(2), Cons(Some(3), Nil)))
    auto three = cons(some(1),
                      cons(some(2),
                           cons(some(3),
                                nil<Option<int>>())));
    
    auto result = ListNode<int>::runOptions(three);
    ASSERT_EQ("Some(Cons(1, Cons(2, Cons(3, Nil))))", toString(result));
  }

  TEST(List, runOptionsFailsOnNone) {
    // Cons(Some(1), Cons(Some(2), Cons(None, Nil)))
    auto three = cons(some(1),
                      cons(some(2),
                           cons(none<int>(),
                                nil<Option<int>>())));
    
    auto result = ListNode<int>::runOptions(three);
    ASSERT_EQ("None", toString(result));
  }

  TEST(List, runIntRdrsSucceeds) {
    auto third = [](int n) { return "c"; };
    auto second = [](int n) { return "b"; };
    auto first = [](int n) { return "a"; };

    auto three = cons(reader<string>(first),
                      cons(reader<string>(second),
                           cons(reader<string>(third),
                                nil<IntRdr<string>>())));
    
    auto result = ListNode<string>::runIntRdrs(three);
    auto list = result->read(7);
    ASSERT_EQ("Cons(a, Cons(b, Cons(c, Nil)))", toString(list));
  }
}

auto main(int argc, char **argv)
  -> int
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
