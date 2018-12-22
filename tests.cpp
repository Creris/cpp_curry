
#include "Curry.hpp"
#include <iostream>

int func_0args() { return 0; }
int func_1args(int a) { std::cout << a << "\n"; return 0; }
int func_2args(int a, int b) { std::cout << a << " " << b << "\n"; return 0; }
int func_3args(int a, int b, int c) { std::cout << a << " " << b << " " << c << "\n"; return 0; }


void runTestsCurried() {
	std::cout << "Testing for ed::curried_function instances\n\n";
	{
		/*
			with values
		*/
		ed::curried_function<int()> curried0{ func_1args, 0 };
		ed::curried_function<int(int)> curried1{ func_2args, 0 };
		ed::curried_function<int(int, int)> curried2{ func_3args, 0 };
		try {
			curried0.curry();
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}
		try {
			ed::curry(curried0, std::make_pair(0, 5));
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}

		auto _1 = curried1.curry(4);
		_1 = ed::curry(curried1, 5);
		//expected: 0 5
		_1();

		auto _2 = curried2.curry(7);
		_1 = _2.curry(8);
		_1 = ed::curry(_2, 9);

		//expected: 0 7 9
		_2(9);
		//expected: 0 7 9
		_1();

		//expected: 0 2 3
		ed::curry(ed::curry(curried2, 2), 3)();
		//expected: 0 5 0 6 0
		ed::curry(ed::curry(curried2, 6), ed::curry(curried1, 5)())();
	}

	{
		/*
			with variables
		*/
		int _t = 6;
		ed::curried_function<int()> curried0{ func_1args, _t };
		ed::curried_function<int(int)> curried1{ func_2args, _t };
		ed::curried_function<int(int, int)> curried2{ func_3args, _t };
		try {
			curried0.curry();
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}
		try {
			ed::curry(curried0, std::make_pair(0, 5));
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}

		auto _1 = curried1.curry(_t - 2);
		_1 = ed::curry(curried1, _t - 1);
		//expected: 6 5
		_1();

		auto _2 = curried2.curry(_t + 3);
		_1 = _2.curry(_t + 8);
		_1 = ed::curry(_2, _t + 2);

		//expected: 6 9 9
		_2(9);
		//expected: 6 9 8
		_1();

		//expected: 6 2 12
		ed::curry(ed::curry(curried2, _t - 4), _t + 6)();
		//expected: 6 7 6 4 0
		ed::curry(ed::curry(curried2, _t - 2), ed::curry(curried1, _t + 1)())();
	}

	{
		/*
			with reference
		*/
		int _t = 6;
		auto _wrap = std::ref(_t);
		ed::curried_function<int()> curried0{ func_1args, std::ref(_t) };
		ed::curried_function<int(int)> curried1{ func_2args, _wrap };
		ed::curried_function<int(int, int)> curried2{ func_3args, std::ref(_wrap) };
		try {
			curried0.curry();
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}
		try {
			ed::curry(curried0, std::make_pair(0, 5));
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}

		auto _1 = curried1.curry(_t - 2);
		_1 = ed::curry(curried1, _t - 1);
		_wrap.get() = 2;
		//expected: 2 5
		_1();

		auto _2 = curried2.curry(_t + 3);
		_1 = _2.curry(_t + 8);
		_1 = ed::curry(_2, _t + 2);
		_wrap.get() = 7;
		//expected: 7 5 9
		_2(9);
		//expected: 7 5 4
		_1();

		//expected: 7 3 7
		ed::curry(ed::curry(curried2, _t - 4), _wrap)();
		//expected: 8 9 8 8 0
		_t = 8;
		ed::curry(ed::curry(curried2, std::ref(_t)), ed::curry(curried1, _t + 1)())();
	}
}

template <class Func0Args, class Func1Arg, class Func2Args>
void runTests(Func0Args f0, Func1Arg f1, Func2Args f2) {
	std::cout << "testing for " << typeid(f0).name() << "\n\n";
	{
		/*
			with values
		*/
		try {
			ed::curry(f0, 0);
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}
		auto _1 = ed::curry(f1, 0);
		auto _2 = ed::curry(f2, 0);
		//expected: 0
		_1();
		//expected: 0 1
		_2(1);

		/*
			with variable
		*/
		int _t = 2;
		try {
			ed::curry(f0, _t);
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}
		auto _11 = ed::curry(f1, _t);
		auto _12 = ed::curry(f2, _t);
		_t = 3;
		//expected: 2
		_11();
		//expected: 2 3
		_12(3);

		/*
			with temporary std::ref
		*/
		try {
			ed::curry(f0, std::ref(_t));
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}
		auto _21 = ed::curry(f1, std::ref(_t));
		auto _22 = ed::curry(f2, std::ref(_t));
		_t = 4;
		//expected: 4
		_21();
		_t = 5;
		//expected: 5 5
		_22(5);

		/*
			with variable of type ref_wrapper
		*/
		auto _wrap = std::ref(_t);
		try {
			ed::curry(f0, std::ref(_t));
		}
		catch (ed::invalid_curry& inc) {
			std::cout << inc.what() << "\n";
		}
		auto _31 = ed::curry(f1, _wrap);
		auto _32 = ed::curry(f2, _wrap);
		_wrap.get() = 6;
		//expected: 6
		_31();
		_wrap.get() = 7;
		//expected: 7 8
		_32(8);
	}
}


void executeTest() {
	/*
		Plain pointers
	*/
	{
		runTests(func_0args, func_1args, func_2args);
	}

	/*
		Function instances
	*/
	{
		std::function<void()> fn0args = []() {};
		std::function<void(int)> fn1args = [](int a) { std::cout << a << "\n"; };
		std::function<void(int, int)> fn2args = [](int a, int b) { std::cout << a << " " << b << "\n"; };

		runTests(fn0args, fn1args, fn2args);
	}

	/*
		Curried function instances
	*/
	{
		runTestsCurried();
	}
}