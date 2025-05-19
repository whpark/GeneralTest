module;

#include <catch.hpp>
#include <glaze/glaze.hpp>

export module reflection;
import std;

namespace test {

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

}
