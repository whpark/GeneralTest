module;

//import <catch.hpp>;

export module constexpr_counter;
import std;

namespace constexpr_counter {

	template<auto T = [] {}>
	constexpr auto unique() {
		return T;
	}

	template<int>
	struct Flag {
		friend constexpr bool flag(Flag);
	};

	template<int N>
	struct Writer {
		friend constexpr bool flag(Flag<N>) {
			return true;
		}
		static constexpr int value = N;
	};

	template<int N = 0>
	constexpr int reader(float) {
		return Writer<N>::value;
	}

	template<int N = 0,
		bool = flag(Flag<N>{}),
		auto = unique()>

	constexpr int reader(int) {
		return reader<N + 1>(int{});
	}


	template<int R = reader<0>(int{})>
	constexpr int next() {
		return R;
	}

	static_assert(next() == 0);
	// NOT WORKING
	//static_assert(next() == 1);
	//static_assert(next() == 2);

}

