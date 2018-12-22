/*
	Compilers supported:
		Visual studio 2015 or newer, _MSC_VER 1900 or higher.
		  As a special note, if _MSC_VER == 1914 or 1915, you need to provide
		  /std:c++17 flag to the compiler. 1916, 1910, 1900 do not require this flag and will
		  compile the code without it.
		GCC 6.1 or newer(-std=c++17 required)
		Clang 5.0.0 or newer(-std=c++17 required)
*/

#ifndef CURRY_HEADER_H_
#define CURRY_HEADER_H_

#include <algorithm>
#include <functional>

/*
	Microsoft specific fix, to support as much stuff as possible.
	In versions lower than 19.14(19.10 tested via Godbolt's website) the compiler
	  does not recognize is_invocable and the macro __cpp_lib_is_invocable is not defined.
	In version 19.14 the functions already exist, but feature test macroes are not yet implemented
	  therefore we have to special case 19.14.
	Newer versions of the compiler(19.15, 19.16 - newest as of the creation) support both the macro and the features.
	As for invoke, we can't check for it(via __cpp_lib_invoke) because older versions dont support feature test macroes,
	but Visual Studio had an implementation for std::invoke for a very long time.
*/
#if !defined __cpp_lib_is_invocable && _MSC_VER != 1914
namespace std {
	template <class Void, class... Args>
	struct _is_invocable_inner : false_type {};

	template <class... Args>
	struct _is_invocable_inner<void_t<decltype(invoke(declval<Args>()...))>, Args...> : true_type {};

	template <class Func, class... Args>
	struct is_invocable : _is_invocable_inner<void, Func, Args...> {};

	template <class Func, class... Args>
	constexpr static bool is_invocable_v = is_invocable<Func, Args...>::value;
}
#endif

namespace ed {
	class invalid_curry final : std::exception {
	public:
		char const* what() const noexcept {
			return "Cannot curry function with 0 arguments";
		}
	};

	template <class X>
	class curried_function;

	template <class Ret>
	class curried_function<Ret()> {
		std::function<Ret()> func;
	public:
		template <class Func, class Arg1, class = 
			std::enable_if_t<std::is_invocable_v<Func, Arg1>, void>
		>
		curried_function(Func fn, const Arg1& arg) : 
			func( [fn, ar = arg]() mutable -> Ret { return fn(std::forward<decltype(ar)>(ar)); } )
		{
		}

		curried_function() noexcept : func(nullptr) {}

		curried_function(const curried_function&) = default;
		curried_function(curried_function&&) = default;
		curried_function& operator=(const curried_function&) = default;
		curried_function& operator=(curried_function&&) = default;
		~curried_function() = default;

		Ret operator()() {
			if (func)
				return func();
			throw std::bad_function_call();
		}

		[[noreturn]] void curry() noexcept(false) {
			throw invalid_curry();
		}

		operator std::function<Ret()>() {
			return func;
		}
	};

	template <class Ret, class Arg0, class... Args>
	class curried_function<Ret(Arg0, Args...)> {
		std::function<Ret(Arg0, Args...)> func;
	public:
		template <class Func, class Arg1, class =
			std::enable_if_t<std::is_invocable_v<Func, Arg1, Arg0, Args...>, void>
		>
		curried_function(Func fn, const Arg1& arg) : func(
			[fn, ar = arg](auto&& arg0, auto&&... args) mutable -> Ret { return fn(std::forward<decltype(ar)>(ar),
																	 std::forward<Arg0>(arg0),
																	 std::forward<Args>(args)...
																	); })
		{
		}

		curried_function() noexcept : func(nullptr) {}

		curried_function(const curried_function&) = default;
		curried_function(curried_function&&) = default;
		curried_function& operator=(const curried_function&) = default;
		curried_function& operator=(curried_function&&) = default;
		~curried_function() = default;

		template <class OpArg0, class... OpArgs>
		Ret operator()(OpArg0&& arg0, OpArgs&&... args) {
			if (func)
				return func(std::forward<Arg0>(arg0), std::forward<Args>(args)...);
			throw std::bad_function_call();
		}

		template <class T>
		curried_function<Ret(Args...)> curry(T&& onValue) {
			return { func, std::forward<T>(onValue) };
		}

		operator std::function<Ret(Arg0, Args...)>() {
			return func;
		}
	};

	/*
		Curry 0 arg functions
	*/
	template <class Ret, class T>
	[[noreturn]] void curry(const curried_function<Ret()>&, const T&) {
		throw invalid_curry();
	}
	template <class Ret, class T>
	[[noreturn]] void curry(const std::function<Ret()>&, const T&) {
		throw invalid_curry();
	}
	template <class Ret, class T>
	[[noreturn]] void curry(Ret(*)(), const T&) {
		throw invalid_curry();
	}
	template <class Func, class T, class = 
		std::enable_if_t<std::is_invocable_v<Func>>
	>
	[[noreturn]] void curry(Func, const T&) {
		throw invalid_curry();
	}


	/*
		Currying curried_function helpers
	*/

	template <class Ret, class Arg1>
	curried_function<Ret()> curry(const curried_function<Ret(Arg1)>& cF, const Arg1& arg) {
		return { cF, arg };
	}
	template <class Ret, class Arg1>
	curried_function<Ret()> curry(const curried_function<Ret(Arg1)>& cF, std::reference_wrapper<Arg1> arg) {
		return { cF, arg };
	}

	template <class Ret, class Arg1, class... Args, class = std::enable_if_t<(sizeof...(Args) > 0), void>>
	curried_function<Ret(Args...)> curry(const curried_function<Ret(Arg1, Args...)>& cF, const Arg1& arg) {
		return { cF, arg };
	}
	template <class Ret, class Arg1, class... Args, class = std::enable_if_t<(sizeof...(Args) > 0), void>>
	curried_function<Ret(Args...)> curry(const curried_function<Ret(Arg1, Args...)>& cF, std::reference_wrapper<Arg1> arg) {
		return { cF, (arg) };
	}


	/*
		Currying std::function helpers
	*/

	template <class Ret, class Arg1>
	curried_function<Ret()> curry(const std::function<Ret(Arg1)>& cF, const Arg1& arg) {
		return { cF, arg };
	}
	template <class Ret, class Arg1>
	curried_function<Ret()> curry(const std::function<Ret(Arg1)>& cF, std::reference_wrapper<Arg1> w) {
		return { cF, w };
	}

	template <class Ret, class Arg1, class... Args, class = std::enable_if_t<(sizeof...(Args) > 0), void>>
	curried_function<Ret(Args...)> curry(const std::function<Ret(Arg1, Args...)>& cF, const Arg1& arg) {
		return { cF, arg };
	}
	template <class Ret, class Arg1, class... Args, class = std::enable_if_t<(sizeof...(Args) > 0), void>>
	curried_function<Ret(Args...)> curry(const std::function<Ret(Arg1, Args...)>& cF, std::reference_wrapper<Arg1> w) {
		return { cF, (w) };
	}


	/*
		Currying plain pointers helpers
	*/

	template <class Ret, class Arg1>
	curried_function<Ret()> curry(Ret(*fn)(Arg1), const Arg1& arg) {
		return { fn, arg };
	}
	template <class Ret, class Arg1>
	curried_function<Ret()> curry(Ret(*fn)(Arg1), std::reference_wrapper<Arg1> w) {
		return { fn, w };
	}

	template <class Ret, class Arg1, class... Args, class = std::enable_if_t<(sizeof...(Args) > 0), void>>
	curried_function<Ret(Args...)> curry(Ret(fn)(Arg1, Args...), const Arg1& arg) {
		return { fn, arg };
	}
	template <class Ret, class Arg1, class... Args, class = std::enable_if_t<(sizeof...(Args) > 0), void>>
	curried_function<Ret(Args...)> curry(Ret(fn)(Arg1, Args...), std::reference_wrapper<Arg1> w) {
		return { fn, (w) };
	}


	/*
		Deduction guides if your compiler supports them
	*/
#if __cpp_deduction_guides
	template <class Ret, class Arg>
	curried_function(std::function<Ret(Arg)>, Arg)->curried_function<Ret()>;
	template <class Ret, class Arg>
	curried_function(Ret(*)(Arg), Arg)->curried_function<Ret()>;
	template <class Ret, class Arg>
	curried_function(curried_function<Ret(Arg)>, Arg)->curried_function<Ret()>;

	template <class Ret, class Arg1, class Arg0, class... Args>
	curried_function(std::function<Ret(Arg1, Arg0, Args...)>, Arg1)->curried_function<Ret(Arg0, Args...)>;
	template <class Ret, class Arg1, class Arg0, class... Args>
	curried_function(Ret(*)(Arg1, Arg0, Args...), Arg1)->curried_function<Ret(Arg0, Args...)>;
	template <class Ret, class Arg1, class Arg0, class... Args>
	curried_function(curried_function<Ret(Arg1, Arg0, Args...)>, Arg1)->curried_function<Ret(Arg0, Args...)>;
#endif
}

#endif	//CURRY_HEADER_H_