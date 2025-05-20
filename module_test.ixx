module;

#include <catch2/catch_all.hpp>

export module module_test;
import :a1;
import :a2;

namespace module_test {

	TEST_CASE("module") {

		xClass1 cl1;
		cl1.m_class2 = std::make_unique<xClass2>();
		REQUIRE(cl1.m_class2->str == "abc");

	}

}
