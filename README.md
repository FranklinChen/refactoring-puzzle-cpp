# Refactoring puzzle

This is an attempt at a `C++` implementation of the [refactoring puzzle by Tony Morris](http://blog.tmorris.net/posts/refactoring-puzzle/index.html) based on his original `C#` implementation.

[`C++11`](http://www.stroustrup.com/C++11FAQ.html) is unapologetically used. There is no reason to suffer with older versions of the `C++` language.

## Setup

The project uses [CMake](http://www.cmake.org/) to build.

[`googletest`](https://code.google.com/p/googletest/) is used for testing. It is not distributed in this repository. You need to download it and put it at subdirectory `gtest`.

To run the tests:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ test/TestRefactoringPuzzle
```

## Notes

### Memory management

I decided it was not worth the trouble to prevent memory leaks in this sample code by coming up with a memory policy, using `std::shared_ptr`, etc.

### Algebraic data types

As with the `C#` code, I use the most "obvious" simulation of algebraic data types using an abstract base class with subclasses.

### No type erasure

Since `C++` templates do not operate by type erasure, virtual member function templates are semantically impossible in `C++`. The simplest (not fastest) workaround was to use `dynamic_cast` to handle all branches from a member function template in the base class.

### Templates

Since templates are not compiled until an attempt at instantiation of use, only exhaustive testing of specific instantiations can provide confidence that templated code actually does what we mean it to do.
