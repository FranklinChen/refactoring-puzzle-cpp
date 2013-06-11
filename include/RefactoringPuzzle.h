// -*- c++ -*-

#ifndef _REFACTORING_PUZZLE_H_
#define _REFACTORING_PUZZLE_H_

#include <functional>
#include <iostream>

// Un-refactored code.
namespace RefactoringPuzzle {
  using namespace std;

  //// IntRdr stuff

  template<typename A> class IntRdrNode;

  template<typename A>
  using IntRdr = shared_ptr<const IntRdrNode<A>>;

  template<typename A>
  class IntRdrNode {
  public:
    function<A(int)> read;
    
    IntRdrNode(function<A(int)> read) : read{read} {}
    
    auto print(ostream &os) const
      -> ostream &
    {
      // Not so informative.
      return os << "IntRdr(" << typeid(read).name() << ")";
    }

    template<typename B>
    auto select(function<B(A)> f) const
      -> IntRdr<B>
    {
      return make_shared<IntRdrNode<B>>
        ([=](int n) { return f(read(n)); });
    }
    
    template<typename B>
    auto selectMany(function<IntRdr<B>(A)> f) const
      -> IntRdr<B>
    {
      return make_shared<IntRdrNode<B>>
        ([=](int n) { return f(read(n))->read(n); });
    }
    
    static auto apply(A a)
      -> IntRdr<A>
    {
      return make_shared<IntRdrNode<A>>
        ([=](int) { return a; });
    }
  };
  
  template<typename A>
  auto reader(function<A(int)> f)
    -> IntRdr<A>
  {
    return make_shared<IntRdrNode<A>>(f);
  }

  //// Option stuff

  template<typename A> class OptionNode;
  template<typename A> class Some;
  template<typename A> class None;

  template<typename A>
  using Option = shared_ptr<const OptionNode<A>>;

  template<typename A>
  auto some(A a) -> Option<A> {
    return make_shared<Some<A>>(a);
  }
    
  template<typename A>
  auto none() -> Option<A> {
    return make_shared<None<A>>();
  }

  template<typename A>
  class OptionNode {
  public:
    virtual ~OptionNode() {}

    virtual auto print(ostream &os) const
      -> ostream & = 0;

    /*
      C# version had an abstract generic method.
      
      It is not possible in C++ to have virtual member function template,
      so we do manual type switch instead in the base class.
    */
    template<typename X>
    auto fold(function<X(A)> some, X none) const
      -> X
    {
      if (auto self = dynamic_cast<const Some<A> *>(this)) {
        return some(self->a);
      }
      else {
        return none;
      }
    }
    
    static auto apply(A a)
      -> Option<A>
    {
      return make_shared<Some<A>>(a);
    }
    
    template<typename B>
    auto select(function<B(A)> f) const
      -> Option<B>
    {
      return fold<Option<B>>([=](A a) {
          return some(f(a));
        },
        none<B>()
        );
    }
    
    template<typename B>
    auto selectMany(function<Option<B>(A)> f) const
      -> Option<B>
    {
      return fold<Option<B>>
        (f, none<B>());
    }
  };
  
  template<typename A>
  class Some : public OptionNode<A> {
  public:
    virtual ~Some() {}
    
    const A a;
    
    Some(A a) : a{a} {}
    
    virtual auto print(ostream &os) const
      -> ostream & override
    {
      return os << "Some(" << a << ")";
    }
  };
  
  template<typename A>
  class None : public OptionNode<A> {
  public:
    virtual ~None() {}
    
    virtual auto print(ostream &os) const
      -> ostream & override
    {
      return os << "None";
    }
  };
  
  //// List stuff

  template<typename A> class ListNode;
  template<typename A> class Cons;
  template<typename A> class Nil;

  template<typename A>
  using List = shared_ptr<const ListNode<A>>;

  template<typename A>
  auto nil() -> List<A> {
    return make_shared<Nil<A>>();
  }

  template<typename A>
  auto cons(A a, List<A> self)
    -> List<A>
  {
    return make_shared<Cons<A>>(a, self);
  }

  template<typename A>
  class ListNode {
  public:
    virtual ~ListNode() {}

    virtual auto print(ostream &os) const
      -> ostream & = 0;

    template<typename X>
    auto foldRight(function<X(A, X)> f, X x) const
      -> X
    {
      if (auto self = dynamic_cast<const Cons<A> *>(this)) {
        return f(self->head, self->tail->foldRight(f, x));
      }
      else {
        return x;
      }
    }

    //// NOTE: duplication in runOptions and runIntRdrs

    // Return all the Some values, or None if not all are Some.
    static auto runOptions(List<Option<A>> x)
      -> Option<List<A>>
    {
      return x->template foldRight<Option<List<A>>>
        ([](Option<A> a,
            Option<List<A>> b) {
          return a->template selectMany<List<A>>([=](A aa) {
              return b->template select<List<A>>([=](List<A> bb) {
                  return cons(aa, bb);
                });
            });
        },
         OptionNode<List<A>>::apply(nil<A>())
         );
    }

    // Apply an Int to a list of int readers and
    // return the list of return values.
    static auto runIntRdrs(List<IntRdr<A>> x)
      -> IntRdr<List<A>>
    {
      return x->template foldRight<IntRdr<List<A>>>
        ([](IntRdr<A> a,
            IntRdr<List<A>> b) {
          return a->template selectMany<List<A>>([=](A aa) {
              return b->template select<List<A>>([=](List<A> bb) {
                  return cons(aa, bb);
                });
            });
        },
         IntRdrNode<List<A>>::apply(nil<A>())
         );
    }
  };

  template<typename A>
  class Nil : public ListNode<A> {
  public:
    virtual ~Nil() {}
    
    virtual auto print(ostream &os) const
      -> ostream & override
      {
        return os << "Nil";
      }
  };
    
  template<typename A>
  class Cons : public ListNode<A> {
  public:
    virtual ~Cons() {}
    
    const A head;
    const List<A> tail;
    
    Cons(A head, List<A> &tail) : head{head}, tail{tail} {}
    
    virtual auto print(ostream &os) const
      -> ostream & override
    {
      return os << "Cons(" << head << ", " << *tail << ")";
    }
  };
  
  //// The rest are operator<<
  template<typename A>
  auto operator<<(ostream &os, const IntRdrNode<A> &reader)
    -> ostream &
  {
    return reader.print(os);
  }

  template<typename A>
  auto operator<<(ostream &os, const OptionNode<A> &option)
    -> ostream &
  {
    return option.print(os);
  }

  template<typename A>
  auto operator<<(ostream &os, const ListNode<A> &list)
    -> ostream &
  {
    return list.print(os);
  }

  // Print what is pointed to.
  template<typename A>
  auto operator<<(ostream &os, const A *a)
    -> ostream &
  {
    return os << *a;
  }
}

#endif
