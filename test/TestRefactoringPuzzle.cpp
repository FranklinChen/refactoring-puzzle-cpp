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
    auto none = make_shared<None<int>>();
    auto result = none->select<int>(add1);
    ASSERT_EQ("None", toString(result));
  }
  
  TEST(Option, selectAdd1OnSome) {
    // Some(4)
    auto someFour = make_shared<Some<int>>(4);
    
    auto result = someFour->select<int>(add1);
    ASSERT_EQ("Some(5)", toString(result));
  }
  
  auto intSomeNil = make_shared<Nil<shared_ptr<const Option<int>>>>();

  auto intSome(int n)
    -> unique_ptr<Some<int>>
  {
    return unique_ptr<Some<int>>(new Some<int>(n));
  }

  auto intNone()
    -> unique_ptr<None<int>>
  {
    return unique_ptr<None<int>>(new None<int>());
  }

  TEST(List, runOptionsSucceedsOnAllSome) {
    // Cons(Some(1), Cons(Some(2), Cons(Some(3), Nil)))
    auto three = intSomeNil->
      prepend(intSome(3))->
      prepend(intSome(2))->
      prepend(intSome(1));
    
    auto result = List<int>::runOptions(three);
    ASSERT_EQ("Some(Cons(1, Cons(2, Cons(3, Nil))))", toString(result));
  }

  TEST(List, runOptionsFailsOnNone) {
    // Cons(Some(1), Cons(Some(2), Cons(None, Nil)))
    auto three = intSomeNil->
      prepend(intNone())->
      prepend(intSome(2))->
      prepend(intSome(1));
    
    auto result = List<int>::runOptions(three);
    
    ASSERT_EQ("None", toString(result));
  }

  auto stringRdrNil = make_shared<Nil<shared_ptr<const IntRdr<string>>>>();

  auto reader(function<string(int)> f)
    -> unique_ptr<IntRdr<string>>
  {
    return unique_ptr<IntRdr<string>>(new IntRdr<string>(f));
  }

  TEST(List, runIntRdrsSucceeds) {
    auto third = [](int n) { return "c"; };
    auto second = [](int n) { return "b"; };
    auto first = [](int n) { return "a"; };

    auto three = stringRdrNil->
      prepend(reader(third))->
      prepend(reader(second))->
      prepend(reader(first));
    
    auto result = List<string>::runIntRdrs(three);
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
