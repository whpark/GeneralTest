#include <fmt/core.h>
#include <fmt/xchar.h>
#include <fmt/chrono.h>

#include <catch.hpp>
#include <boost/endian.hpp>
#include "boost/pfr.hpp"
#include <lzma.h>

#include <units.h>

import std;

using namespace std::literals;
using namespace units::literals;

namespace biscuit {
	class xBase {
	public:
		using this_t = xBase;
	public:
		std::string name;

		virtual ~xBase() = default;

		virtual std::unique_ptr<xBase> clone() const = 0;

	};

	struct xOther {
		std::string name;
	};

	struct xOther2 {
		std::string name;
	};

	struct sInter {
		xOther other;
		xOther2 other2;
	};

	class xDerived1 : public xBase, public sInter {
	public:
		using this_t = xDerived1;
		using base_t = xBase;

		virtual ~xDerived1() = default;

		std::unique_ptr<xBase> clone() const override {
			return std::make_unique<xDerived1>(*this);
		}

	};

	class xDerived2 : public xBase {
	public:
		virtual ~xDerived2() = default;

		std::unique_ptr<xBase> clone() const override {
			return std::make_unique<xDerived2>(*this);
		}

	};

	TEST_CASE("poly") {
		std::vector<std::unique_ptr<xBase>> lst;

		lst.emplace_back(std::make_unique<xDerived1>());
		lst.emplace_back(std::make_unique<xDerived2>());
		if (auto* p = dynamic_cast<xDerived1*>(lst[0].get())) {
			p->other.name = "other";
			p->other2.name = "other2";
			lst.push_back(p->clone());
		}

		xDerived1* ptr = dynamic_cast<xDerived1*>(lst[2].get());
		REQUIRE(ptr);
		REQUIRE(ptr->other.name == "other"s);
		REQUIRE(ptr->other2.name == "other2"s);

	}


}
