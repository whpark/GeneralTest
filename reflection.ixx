module;

#include <catch.hpp>
#include <glaze/glaze.hpp>

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

	template < xStringLiteral name >
	struct sBase {
		std::string str{name.str};
		int a{};
	};

	sBase<"sdfds"> base;

}
