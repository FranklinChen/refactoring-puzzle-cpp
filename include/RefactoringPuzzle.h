// -*- c++ -*-

#ifndef _REFACTORING_PUZZLE_H_
#define _REFACTORING_PUZZLE_H_

#include <functional>
#include <iostream>

namespace RefactoringPuzzle {
  template<typename A>
  class IntRdr {
  public:
    ::std::function<A(int)> read;
    
    IntRdr(::std::function<A(int)> read) : read(read) {}
    
    template<typename B>
    auto select(::std::function<B(A)> f)
      -> IntRdr<B> *
    {
      return new IntRdr<B>([this, f](int n) { return f(read(n)); });
    }
    
    template<typename B>
    auto selectMany(::std::function<IntRdr<B> *(A)> f)
      -> IntRdr<B> *
    {
      return new IntRdr<B>([this, f](int n) { return f(read(n))->read(n); });
    }
    
    static auto apply(A a)
      -> IntRdr *
    {
      return new IntRdr<A>([a](int) { return a; });
    }

    auto print(::std::ostream &os) const
      -> ::std::ostream &
    {
      // Not so informative.
      return os << "IntRdr(" << typeid(read).name() << ")";
    }
  };
  
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
    auto fold(::std::function<X(A)> some, X none)
      -> X
    {
      if (auto self = dynamic_cast<Some *>(this)) {
        return some(self->a);
      }
      else {
        return none;
      }
    }
    
    static auto apply(A a)
      -> Option *
    {
      return new Some(a);
    }
    
    virtual auto print(::std::ostream &os) const
      -> ::std::ostream & = 0;

    class Some : public Option {
    public:
      A const a;
      
      Some(A a) : a(a) {}

      virtual auto print(::std::ostream &os) const
        -> ::std::ostream & override
      {
        return os << "Some(" << a << ")";
      }

    };
    
    class None : public Option {
    public:
      virtual auto print(::std::ostream &os) const
        -> ::std::ostream & override
      {
        return os << "None";
      }
    };
    
    template<typename B>
    auto select(::std::function<B(A)> f)
      -> Option<B> *
    {
      return fold<Option<B> *>([f](A a) {
          return new typename Option<B>::Some(f(a));
        },
        new typename Option<B>::None());
    }
    
    template<typename B>
    auto selectMany(::std::function<Option<B> *(A)> f)
      -> Option<B> *
    {
      return fold<Option<B> *>(f,
                               new typename Option<B>::None());
    }
  };
  
  template<typename A>
  class List {
  public:
    virtual ~List() {}

    template<typename X>
    auto foldRight(::std::function<X(A, X)> f, X x)
      -> X
    {
      if (auto self = dynamic_cast<Cons *>(this)) {
        return f(self->head, self->tail->foldRight(f, x));
      }
      else {
        return x;
      }
    }

    //// NOTE: duplication in runOptions and runIntRdrs

    // Return all the Some values, or None if not all are Some.
    static auto runOptions(List<Option<A> *> *x)
      -> Option<List *> *
    {
      return x->template foldRight<Option<List *> *>
        ([](Option<A> *a,
            Option<List *> *b) {
          return a->template selectMany<List *>([b](A aa) {
              return b->template select<List *>([aa](List *bb) {
                  return bb->prepend(aa);
                });
            });
        },
         Option<List *>::apply(new Nil()));
    }

    // Apply an Int to a list of int readers and
    // return the list of return values.
    static auto runIntRdrs(List<IntRdr<A> *> *x)
      -> IntRdr<List *> *
    {
      return x->template foldRight<IntRdr<List *> *>
        ([](IntRdr<A> *a,
            IntRdr<List *> *b) {
          return a->template selectMany<List *>([b](A aa) {
              return b->template select<List *>([aa](List *bb) {
                  return bb->prepend(aa);
                });
            });
        },
         IntRdr<List *>::apply(new Nil()));
    }
    
    auto prepend(A a)
      -> List *
    {
      return new Cons(a, this);
    }
    
    virtual auto print(::std::ostream &os) const
      -> ::std::ostream & = 0;

    class Nil : public List {
    public:
      virtual auto print(::std::ostream &os) const
        -> ::std::ostream & override
      {
        return os << "Nil";
      }
    };
    
    class Cons : public List {
    public:
      A const head;
      List *const tail;
      
      Cons(A head, List *tail) : head(head), tail(tail) {}

      virtual auto print(::std::ostream &os) const
        -> ::std::ostream & override
      {
        return os << "Cons(" << head << ", " << *tail << ")";
      }
    };
  };

  //// The rest are operator<<
  template<typename A>
  auto operator<<(::std::ostream &os, const IntRdr<A> &reader)
    -> ::std::ostream &
  {
    return reader.print(os);
  }

  template<typename A>
  auto operator<<(::std::ostream &os, const Option<A> &option)
    -> ::std::ostream &
  {
    return option.print(os);
  }

  template<typename A>
  auto operator<<(::std::ostream &os, const List<A> &list)
    -> ::std::ostream &
  {
    return list.print(os);
  }

  // Print what is pointed to.
  template<typename A>
  auto operator<<(::std::ostream &os, const A *a)
    -> ::std::ostream &
  {
    return os << *a;
  }
}

#endif
