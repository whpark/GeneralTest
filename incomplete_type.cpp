#include <catch.hpp>
#include "incomplete_type.h"

import std;

namespace incomplete_type {

	xTesterDest::~xTesterDest() {
	}

	class xIncompleteType {
	public:
		std::string name;
		xIncompleteType(std::string str) : name(std::move(str)) {
			std::println("xIncompleteType: {}", name);
		}
		~xIncompleteType() {
			std::println("~xIncompleteType: {}", name);
		}
	};

	TEST_CASE("incomplete_type", "[incomplete_type]") {
		std::println("===========");
		{
			xTesterDefaultDest tester1;
			tester1.m_ptr = std::make_unique<xIncompleteType>("xTesterDefaultDest");
		}
		{
			xTesterDestInline tester2;
			tester2.m_ptr = std::make_unique<xIncompleteType>("xTesterDestInline");
		}
		{
			xTesterDest tester3;
			tester3.m_ptr = std::make_unique<xIncompleteType>("xTesterDest");
		}
	}

};
