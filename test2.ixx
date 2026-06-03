module;

#include <fmt/core.h>
#include <fmt/xchar.h>
#include <fmt/chrono.h>

#include <catch.hpp>
#include <boost/endian.hpp>
#include <lzma.h>

export module test2;
import std;

using namespace std::literals;

export namespace test2 {

	template < typename T, int DEFAULT_FRONT = -1, int DEFAULT_BACK = 1 >
	struct TRECT3 {
		T left{}, top{}, front{DEFAULT_FRONT}, right{}, bottom{}, back{DEFAULT_BACK};
		auto operator <=> (TRECT3 const&) const = default;
	};

}
