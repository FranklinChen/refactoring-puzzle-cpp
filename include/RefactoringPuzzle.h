// -*- c++ -*-

#ifndef _REFACTORING_PUZZLE_H_
#define _REFACTORING_PUZZLE_H_

#include <functional>
#include <iostream>

namespace RefactoringPuzzle {
  using namespace std;

  template<typename A>
  class IntRdr {
  public:
    function<A(int)> read;
    
    IntRdr(function<A(int)> read) : read{read} {}
    
    template<typename B>
    auto select(function<B(A)> f) const
      -> unique_ptr<const IntRdr<B>>
    {
      return unique_ptr<const IntRdr<B>>
        (new IntRdr<B>([this, f](int n) { return f(read(n)); }));
    }
    
    // TODO too restrictive?
    template<typename B>
    auto selectMany(function<unique_ptr<const IntRdr<B>>(A)> f) const
      -> unique_ptr<const IntRdr<B>>
    {
      return unique_ptr<const IntRdr<B>>
        (new IntRdr<B>([this, f](int n) { return f(read(n))->read(n); }));
    }
    
    static auto apply(A a)
      -> unique_ptr<const IntRdr>
    {
      return unique_ptr<const IntRdr>
        (new IntRdr<A>([a](int) { return a; }));
    }

    auto print(ostream &os) const
      -> ostream &
    {
      // Not so informative.
      return os << "IntRdr(" << typeid(read).name() << ")";
    }
  };
  
  template<typename A> class Some;
  template<typename A> class None;

  template<typename A>
  class Option {
  public:
    virtual ~Option() {}

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
      -> unique_ptr<const Option>
    {
      return unique_ptr<const Option>(new Some<A>(a));
    }
    
    virtual auto print(ostream &os) const
      -> ostream & = 0;

    // TODO
    template<typename B>
    auto select(function<B(A)> f) const
      -> shared_ptr<const Option<B>>
    {
      return fold<unique_ptr<const Option<B>>>([f](A a) {
          return unique_ptr<const Some<B>>(new Some<B>(f(a)));
        },
        unique_ptr<const None<B>>(new None<B>()));
    }
    
    template<typename B>
    auto selectMany(function<shared_ptr<const Option<B>>(A)> f) const
      -> shared_ptr<const Option<B>>
    {
      return fold<shared_ptr<const Option<B>>>
        (f, unique_ptr<const None<B>>(new None<B>()));
    }
  };
  
  template<typename A>
  class Some : public Option<A> {
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
  class None : public Option<A> {
  public:
    virtual ~None() {}
    
    virtual auto print(ostream &os) const
      -> ostream & override
    {
      return os << "None";
    }
  };
  

  template<typename A> class Cons;
  template<typename A> class Nil;

  template<typename A>
  class List {
  public:
    using Ptr = shared_ptr<const List>;

    virtual ~List() {}

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
    static auto runOptions(shared_ptr<const List<shared_ptr<const Option<A>>>> x)
      -> shared_ptr<const Option<shared_ptr<const List>>>
    {
      return x->template foldRight<shared_ptr<const Option<shared_ptr<const List>>>>
        ([](shared_ptr<const Option<A>> a,
            shared_ptr<const Option<shared_ptr<const List>>> b) {
          return a->template selectMany<shared_ptr<const List>>([b](A aa) {
              return b->template select<shared_ptr<const List>>([aa](shared_ptr<const List> bb) {
                  return bb->prepend(aa);
                });
            });
        },
         Option<shared_ptr<const List>>::apply(make_shared<Nil<A>>()));
    }

    // Apply an Int to a list of int readers and
    // return the list of return values.
    static auto runIntRdrs(shared_ptr<const List<shared_ptr<const IntRdr<A>>>> x)
      -> shared_ptr<const IntRdr<shared_ptr<const List>>>
    {
      return x->template foldRight<shared_ptr<const IntRdr<shared_ptr<const List>>>>
        ([](shared_ptr<const IntRdr<A>> a,
            shared_ptr<const IntRdr<shared_ptr<const List>>> b) {
          return a->template selectMany<shared_ptr<const List>>([b](A aa) {
              return b->template select<shared_ptr<const List>>([aa](shared_ptr<const List> bb) {
                  return bb->prepend(aa);
                });
            });
        },
         IntRdr<shared_ptr<const List>>::apply(make_shared<Nil<A>>()));
    }
    
    auto prepend(A a) const
      -> shared_ptr<const List>
    {
      // Have to explicitly wrap this in shared_ptr
      return make_shared<Cons<A>>(a, shared_ptr<const List>(this));
    }
    
    virtual auto print(ostream &os) const
      -> ostream & = 0;

  };

  template<typename A>
  class Nil : public List<A> {
  public:
    virtual ~Nil() {}
    
    virtual auto print(ostream &os) const
      -> ostream & override
      {
        return os << "Nil";
      }
  };
    
  template<typename A>
  class Cons : public List<A> {
  public:
    virtual ~Cons() {}
    
    const A head;
    const shared_ptr<const List<A>> tail;
    
    Cons(A head, const shared_ptr<const List<A>> &tail) : head{head}, tail{tail} {}
    
    virtual auto print(ostream &os) const
      -> ostream & override
    {
      return os << "Cons(" << head << ", " << *tail << ")";
    }
  };
  
  //// The rest are operator<<
  template<typename A>
  auto operator<<(ostream &os, const IntRdr<A> &reader)
    -> ostream &
  {
    return reader.print(os);
  }

  template<typename A>
  auto operator<<(ostream &os, const Option<A> &option)
    -> ostream &
  {
    return option.print(os);
  }

  template<typename A>
  auto operator<<(ostream &os, const List<A> &list)
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
