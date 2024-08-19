#include "catch.hpp"

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>

import std.compat;

namespace test {

	template < typename T >
	struct TPoint {
		T x, y;

		//friend class cereal::access;

		template < class Archive >
		void serialize(Archive& ar) {
			ar(x, y);
		}
	};
	//CEREAL_REGISTER_TYPE(Point<int>);
	//CEREAL_REGISTER_TYPE(Point<float>);
	//CEREAL_REGISTER_TYPE(Point<double>);
}

//CEREAL_REGISTER_TYPE(test::Point<int>);
//CEREAL_REGISTER_TYPE(test::Point<float>);
//CEREAL_REGISTER_TYPE(test::Point<double>);

namespace test {
	TEST_CASE("Cereal_templ", "[Cereal]") {
		{
			std::ofstream f("cereal_templ.bin");
			cereal::BinaryOutputArchive ar(f);
			TPoint<int> p{ 1, 2 };
			TPoint<double> p2{ 1.2, 2.3 };
			ar(p, p2);
		}
		{
			std::ifstream f("cereal_templ.bin");
			cereal::BinaryInputArchive ar(f);
			TPoint<int> p{};
			TPoint<double> p2{};
			ar(p, p2);
			REQUIRE(p.x == 1);
			REQUIRE(p.y == 2);
			REQUIRE(p2.x == 1.2);
			REQUIRE(p2.y == 2.3);
		}
	}
}
