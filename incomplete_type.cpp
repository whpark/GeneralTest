#include <catch.hpp>
#include "incomplete_type.h"

import std;

namespace incomplete_type {

	class xIncompleteType {
	public:
		std::string name;
		xIncompleteType() {
			std::println("xIncompleteType constructor");
		}
		~xIncompleteType() {
			std::println("xIncompleteType destructor");
		}
	};

	xIncompleteTypeDefaultDestImpl::xIncompleteTypeDefaultDestImpl() = default;
	xIncompleteTypeDefaultDestImpl::~xIncompleteTypeDefaultDestImpl() = default;
	xIncompleteTypeDest::xIncompleteTypeDest() { }
	xIncompleteTypeDest::~xIncompleteTypeDest() { }

	TEST_CASE("incomplete_type", "[incomplete_type]") {
		std::println("===========");
		xTester tester;
		tester.m_ptr = std::make_unique<xIncompleteType>();
		tester.m_ptrDefaultDest = std::make_unique<xIncompleteTypeDefaultDest>();
		tester.m_ptrDefaultDestImpl = std::make_unique<xIncompleteTypeDefaultDestImpl>();
		tester.m_ptrDestInline = std::make_unique<xIncompleteTypeDestInline>();
		tester.m_ptrDest = std::make_unique<xIncompleteTypeDest>();
	}



};
