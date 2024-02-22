#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <fmt/core.h>
#include <fmt/xchar.h>

#include <catch.hpp>

#include <boost/endian.hpp>

using namespace std::literals;

namespace test {

	struct normal {
		uint8_t foo;
		uint32_t bar;
	};
	static_assert(sizeof(normal) == 8);

	struct packed {
		uint8_t foo;
		boost::endian::native_uint32_t bar;
	};
	static_assert(sizeof(packed) == 5);

	TEST_CASE("misc", "[nth_element]") {
		std::vector<int> data{ 8, 1, 9, 2, 7, 3, 6, 4, 5 };
		std::nth_element(data.begin(), data.begin() + 4, data.end());
		REQUIRE(data[4] == 5);
		//std::cout << "data[4] == " << data[4] << "\n";

		std::nth_element(data.begin(), data.begin() + 7, data.end(),
						 std::greater<>());
		REQUIRE(data[7] == 2);
		//std::cout << "data[7] == " << data[7] << "\n";
	}

	class xA {
	public:
		std::string str;

		xA(std::string str) : str(std::move(str)) {}
		//xA(xA const& other) : str(other.str) {}
		//xA(xA&& other) : str(std::move(other.str)) {}
		//xA& operator=(xA const& other) {
		//	str = other.str;
		//	return *this;
		//}
		//xA& operator=(xA&& other) {
		//	str = std::move(other.str);
		//	return *this;
		//}
	};

	template < size_t N >
	class xStringLiteral {
	public:
		char content[N];
		static const auto length = N-1;
		constexpr xStringLiteral(char const (&str)[N]) {
			for (auto [i, c] : std::ranges::enumerate_view(str))
				content[i] = c;
		}
	};

	template < xStringLiteral lit >
	constexpr auto operator "" _lit() { return lit; }

	template < typename tchar_to, xStringLiteral literal >
	struct TStringLiteral {
		tchar_to value[std::size(literal.content)];
		static const auto length = literal.length;
		constexpr TStringLiteral() {
			for (size_t i{}; i < std::size(literal.content); i++) {
				//static_assert(literal.str[i] > 0 and literal.str[i] < 127);
				if (literal.content[i] < 0 or literal.content[i] > 127)
					throw std::logic_error("invalid character");
				value[i] = static_cast<tchar_to>(literal.content[i]);
			}
		}
	};

	template < xStringLiteral source >
	constexpr auto operator "" _ToW() {
		static constexpr TStringLiteral<wchar_t, source> lit;
		return lit.value;
	}

	template < xStringLiteral source, typename tchar_to >
	constexpr auto ToString() {
		static constexpr TStringLiteral<tchar_to, source> lit;
		return lit.value;
	}

	TEST_CASE("misc", "[rule_of_five]") {
		xA a("hello");
		xA b = a;
		REQUIRE(b.str == "hello");
		xA c = std::move(a);
		REQUIRE(c.str == "hello");
		REQUIRE(a.str == "");

		constexpr xStringLiteral strA("hello, world!");
		constexpr auto strB = "hello, world!"_lit;

		//constexpr auto strW = TStringLiteral<wchar_t, strA>{};
		constexpr auto strW = TStringLiteral<wchar_t, "Hello World!"_lit>{};
		static std::wstring const str2 (L"Sdfasdf");
		REQUIRE(str2 == strW.value);
		REQUIRE(str2 == L"Hello World!");
		fmt::print(L"str: {}\n", strW.value);

		REQUIRE("ABC"_ToW == L"ABC"s);
		REQUIRE(ToString<"ABC"_lit, wchar_t>() == L"ABC"s);
	}

}