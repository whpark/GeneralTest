#include <fmt/core.h>
#include <fmt/xchar.h>
#include <fmt/chrono.h>

#include <catch.hpp>
#include <boost/endian.hpp>
#include <lzma.h>

import std;
import test2;

using namespace std::literals;

namespace test2 {

	TEST_CASE("Rect3_Spaceship") {
		TRECT3<int> rect1{1, 2, 3, 4, 5, 6};
		TRECT3<int> rect2{1, 2, 3, 4, 5, 6};
		//TRECT3<int, 0, 1> rect3{1, 2, -1, 4, 5, -1};
		TRECT3<int> rect3{1, 2, -1, 4, 5, -1};
		REQUIRE(rect1 == rect2);
		REQUIRE(rect1 != rect3);
	}

}
