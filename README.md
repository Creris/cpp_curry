# cpp_curry

This library allows users to do a so-called "currying" to any callable object in C++.

You can curry functions until they take no parameters and return their return value immediately after calling. Trying to curry a function with no parameters will cause an exception of type `ed::invalid_curry` to be raised.

Every curry call has the same effect as if you called `std::bind` on the result of the last `std::bind`(or function directly, if none) while binding a single value to the callable object.

This library provides optional, feature macro guarded deduction guides, which detect the type of the curried type automatically from parameters from the contructor. These deductions do not work on lambda objects, which have to provide template arguments explicitly, or have to turn the lambda into a function using `+` trick.

This library provides two ways to curry a function.

1. A class `ed::curried_function<Return(Arguments...)>` class, which provides a constructor with signature `curried_function(Callable, const FirstArg&)` and turns the function `Callable`(which is assumed to be with signature `(FirstArg, Arguments...)`) and have return type `Return`.
2. A `curry` member function of class `curried_function`, which will curry given instance with a parameter, returning instance of new type of `curried_function` with the first argument from the list removed.
3. A free standing overloaded functions `ed::curry`, which take various callable objects and perform currying on top of them as if by constructing new instance of `curried_function` with that parameter. Please note that you can't pass a lambda into `curry` free function.

If you wish to pass variables into the curried objects as references, you can use `std::reference_wrapper` and the associated maker function `std::ref`.

## Compiler support

This library can be compiled with following compilers:

* GCC 6.1 or newer
* Clang 5.0.0 or newer
  * Both these compilers require the flag `-std=c++17` for this library to compile
* Visual Studio 2015 or newer
  * The compiler version needs to be at least 19.10(`_MSC_VER > 1910`)
  * Visual studio does not require any language version flag to be passed.
    * unless you are using compiler version 19.14 or 19.15. On these versions for some reason the compiler will complain about constructors not existing and will therefore require `/std:c++17` flag to be specified.

## API Overview

```cpp

namespace ed {
    class invalid_curry final : std::exception;

    template <class Ret, class... Args>
    class curried_function {
        template <class Func, class Arg1>
        curried_function(Func fn, const Arg1& arg);

        curried_function() noexcept;
        curried_function(const curried_function&) = default;
        curried_function(curried_function&&) = default;
        curried_function& operator=(const curried_function&) = default;
        curried_function& operator=(curried_function&&) = default;
        ~curried_function() = default;

        //Call this curried function
        template <class OpArg0, class... OpArgs>
        Ret operator()(OpArg0&& arg0, OpArgs&&... args);

        //curry this instance, creating new instance with one less parameter
        template <class T>
        curried_function<Ret(Args...)> curry(T&& onValue);

        //conversion operator to std::function
        operator std::function<Ret(Arg0, Args...)>();
    };

    //These always throw, you cannot curry function with no arguments
    template <class Ret, class T>
    [[noreturn]] void curry(const curried_function<Ret()>&, const T&);
    template <class Ret, class T>
    [[noreturn]] void curry(const std::function<Ret()>&, const T&);
    template <class Ret, class T>
    [[noreturn]] void curry(Ret(*)(), const T&);
    template <class Func, class T, class>
    [[noreturn]] void curry(Func, const T&);

    //Takes a callable object(std::function, function pointer, curried_function)
    //and returns new, more curried function
    template <class Func, class Arg>
    auto curry(Func f, const Arg& firstArg);

#if __cpp_deduction_guides
    //Deduction guides
#endif
}
```

## Testing file `tests.cpp`

This file is provided for some very simple test(the expected values are actually in comments, so its not much of a test in reality). This file is more of a compilation of all the possible API calls, verifying that the library will successfully build.

You can refer to this file if you want to see example of how this library is used.