#include <iostream>
#include "RefactoringPuzzle.h"

#include "gtest/gtest.h"

namespace TestRefactoringPuzzle {
  // Shortcut for testing: do not bother to implement equality on
  // our data structures, but just look at the string representations!

  using namespace RefactoringPuzzle;

  template<typename A>
  auto toString(const A &a)
    -> ::std::string
  {
    ::std::stringstream s;
    s << a;
    return s.str();
  }

  using IntOption = Option<int>;
  using IntSome = IntOption::Some;
  using IntNone = IntOption::None;
  
  auto add1 = [](int n) { return n+1; };

  TEST(Option, selectAdd1OnNome) {
    auto result = (new IntNone())->select<int>(add1);
    ASSERT_EQ("None", toString(result));
  }
  
  TEST(Option, selectAdd1OnSome) {
    // Some(4)
    auto someFour = new IntSome(4);
    
    auto result = someFour->select<int>(add1);
    ASSERT_EQ("Some(5)", toString(result));
  }
  
  using IntList = List<int>;
  using IntOptionList = List<IntOption *>;
  
  TEST(List, runOptionsSucceedsOnAllSome) {
    // Cons(Some(1), Cons(Some(2), Cons(Some(3), Nil)))
    auto three = (new IntOptionList::Nil())->
      prepend(new IntSome(3))->
      prepend(new IntSome(2))->
      prepend(new IntSome(1));
    
    auto result = IntList::runOptions(three);
    ASSERT_EQ("Some(Cons(1, Cons(2, Cons(3, Nil))))", toString(result));
  }
  
  TEST(List, runOptionsFailsOnNone) {
    // Cons(Some(1), Cons(Some(2), Cons(None, Nil)))
    auto three = (new IntOptionList::Nil())->
      prepend(new IntOption::None())->
      prepend(new IntSome(2))->
      prepend(new IntSome(1));
    
    auto result = IntList::runOptions(three);
    
    ASSERT_EQ("None", toString(result));
  }

  using StringList = List<::std::string>;
  using StringRdr = IntRdr<::std::string>;
  using StringRdrList = List<StringRdr *>;

  TEST(List, runIntRdrsSucceeds) {
    auto third = [](int n) { return "c"; };
    auto second = [](int n) { return "b"; };
    auto first = [](int n) { return "a"; };

    auto three = (new StringRdrList::Nil())->
      prepend(new StringRdr(third))->
      prepend(new StringRdr(second))->
      prepend(new StringRdr(first));
    
    auto result = StringList::runIntRdrs(three);
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
