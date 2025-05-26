module;

#include <catch.hpp>
#include <glaze/glaze.hpp>
#include <boost/pfr.hpp>

export module reflection;
import std;
using namespace std::literals;

namespace test {

	template < size_t N >
	class xStringLiteral {
	public:
		char str[N];
		constexpr xStringLiteral(char const (&sz)[N]) {
			std::copy_n(sz, N, this->str);
		}
	};

	class xBase {
	public:
		std::string str;
		int a{};

	public:
		//virtual ~xBase() = default;
		//virtual void foo() = 0;
	};

	static_assert(glz::reflect<xBase>::size == 2);
	static_assert(glz::reflect<xBase>::keys[0] == "str");

	template < typename T = xBase >
	void Func(T field) {
		//constexpr static auto const size = glz::reflect<T>::size;

	}


	template < xStringLiteral name >
	struct sBase {
		std::string str{name.str};
		int a{};
	};

	sBase<"sdfds"> base;

}

namespace test_reflection {

	struct any_t final {
		template <class T>
		constexpr operator T() const;
	};

	template <class T, class... Args>
		requires(std::is_aggregate_v<std::remove_cvref_t<T>>)
	inline constexpr auto count_members = [] {
		using V = std::remove_cvref_t<T>;
		if constexpr (requires { V{Args{}..., any_t{}}; }) {
			return count_members<V, Args..., any_t>;
		}
		else {
			return sizeof...(Args);
		}
	}();
	constexpr size_t max_pure_reflection_count = 128;

	template < typename T >
	struct TCountStructMember;

	//template < typename ... TArgs >
	//struct TCountStructMember<struct{TArgs ...}> {
	//	constexpr static auto const size = sizeof...(TArgs);
	//};

	namespace detail {
		// A stand-in object that can convert to anything implicitly.
		template<std::size_t> struct any { template<class T> constexpr operator T() const noexcept; };

		// true when T can be brace-initialized with N 'any' arguments
		template<class T, std::size_t... Is>
		constexpr bool brace_constructible(std::index_sequence<Is...>) noexcept {
			return std::is_constructible_v<T, any<Is>...>;
		}

		template<class T, std::size_t N>
		constexpr bool brace_constructible_n =
			brace_constructible<T>(std::make_index_sequence<N>{});

		// -----------------------------------------------------------------
		// constexpr function that returns the number of *public* data members
		// (works for aggregates only)
		template<class T, std::size_t N> requires (std::is_aggregate_v<T>)
		constexpr std::size_t field_count() noexcept {
			if constexpr (brace_constructible_n<T, N + 1>)
				return field_count<T, N + 1>();        // tail recursion, folds at compile time
			else
				return N;                              // N is the first size that fails
		}

		template < class T >
		auto tie(T& v) {
			static constexpr size_t N = field_count<T, 0>();
			if constexpr (N == 0) { return std::tuple{}; }
			else if constexpr (N == 1) { auto& [a] = v; return std::tuple{&a}; }
			else if constexpr (N == 2) { auto& [a, b] = v; return std::tuple{&a, &b}; }
			else if constexpr (N == 3) { auto& [a, b, c] = v; return std::tuple{&a, &b, &c}; }
			else if constexpr (N == 4) { auto& [a, b, c, d] = v; return std::tuple{&a, &b, &c, &d}; }
			else if constexpr (N == 5) { auto& [a, b, c, d, e] = v; return std::tuple{&a, &b, &c, &d, &e}; }
			else if constexpr (N == 6) { auto& [a, b, c, d, e, f] = v; return std::tuple{&a, &b, &c, &d, &e, &f}; }
			else if constexpr (N == 7) { auto& [a, b, c, d, e, f, g] = v; return std::tuple{&a, &b, &c, &d, &e, &f, &g}; }
			else static_assert(false);

		}

		//template < typename T >
		//constexpr auto CountStructMember() {
		//	if constexpr (requires (T a) { auto [b] = a; }) { return 1; }
		//	else if constexpr (requires (T a) { auto [b, c] = a; }) { return 2; }
		//	else if constexpr (requires (T a) { auto [b, c, d] = a; }) { return 3; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e] = a; }) { return 4; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f] = a; }) { return 5; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f, g] = a; }) { return 6; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f, g, h] = a; }) { return 7; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f, g, h, i] = a; }) { return 8; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f, g, h, i, j] = a; }) { return 9; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f, g, h, i, j, k] = a; }) { return 10; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f, g, h, i, j, k, l] = a; }) { return 11; }
		//	else if constexpr (requires (T a) { auto [b, c, d, e, f, g, h, i, j, k, l, m] = a; }) { return 12; }
		//	else if constexpr (requires (T a) { auto [v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12] = a;}) { return 13; }
		//	else static_assert(false);
		//}

	} // namespace detail

	template<class T>
	constexpr std::size_t field_count() noexcept {
		return detail::field_count<T, 0>();
	}

	template < size_t I, typename T >
	auto& get(T& v) {
		return *std::get<I>(detail::tie<T>(v));
	}

	struct sBase {
		int k;

	};

	struct sTest {
		int a{};
		float b{};
		double c{};
		double* ptr{};
		std::string str{};
		std::unique_ptr<sTest> next{};
	};

	struct sDerived : public sBase {
		int a{};
		float b{};
		double c{};
		double* ptr{};
		std::string str{};
		std::unique_ptr<sTest> next{};

	};

	template < typename T >
	void Func2(T& v) {
		constexpr auto N = boost::pfr::tuple_size_v<T>;
		boost::pfr::get<0>(v) = 42;
	}

	template < typename T >
	void Func3(T& v) {
		//constexpr auto N = glz::reflect<T>::size;
	}

	//static_assert(field_count<sTest>() == 6);
	static_assert(field_count<sBase>() == 1);
	constexpr auto N = field_count<sDerived>();
	TEST_CASE("field_count", "reflection") {
		std::println("field count : {}", N);
		sTest test;
		//boost::pfr::get<0>(test) = 3;
		get<0>(test) = 3;
		REQUIRE(test.a == 3);

		sTest test2;
		Func2(test2);
		REQUIRE(test2.a == 42);
		Func3(test2);
		//REQUIRE(test2.a == 43);
		//REQUIRE(N == 7);
	}
	//static_assert(field_count<sDerived>() == 7);


	static_assert(boost::pfr::tuple_size_v<sTest> == 6);

	static_assert(boost::pfr::tuple_size_v<sTest> == 6);

	static_assert(6 == count_members<sTest>);


}
