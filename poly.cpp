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

namespace test_poly {

	struct sBase {
		std::string name;

		auto operator<=>(sBase const& other) const = default;

		template < typename tSelf >
		auto& base_data(this tSelf&& self) {
			using T = std::conditional_t<
				std::is_const_v<std::remove_reference_t<tSelf>>,
				sBase const,
				sBase>;
			return static_cast<T&>(self);
		}
	};

	class xBase : public sBase {
	public:
		std::vector<std::unique_ptr<xBase>> children;

	public:
		xBase() = default;
		xBase(xBase const&) = delete;
		xBase(xBase&&) = default;
		xBase& operator = (xBase const&) = delete;
		xBase& operator = (xBase&&) = default;
		xBase(std::string name) : sBase{std::move(name)} {}

		bool operator == (xBase const& other) const {
			return base_data() == other.base_data()
				&& children.size() == other.children.size()
				&& std::equal(children.begin(), children.end(), other.children.begin(),
					[](auto const& left,  auto const& right) {
						if (left and right)
							return *left == *right;
						return !left and !right;
			});
		}
	};

	// 이 함수 적용 안됨
	std::weak_ordering operator <=> (std::unique_ptr<xBase> const& left, std::unique_ptr<xBase> const& right) {
		if (left && right)
			return *left <=> *right;
		if (!left && !right)
			return 1 <=> 1;
		return left ? 1 <=> 0 : 0 <=> 1;
	}

	class xDerived1 : public xBase {
	public:
		int a{};

		auto operator<=>(xDerived1 const& other) const = default;
	};

	TEST_CASE("compare", "[poly]") {
		xDerived1 d1;
		d1.name = "node1";
		d1.a = 42;
		xDerived1 d2;
		d2.name = "node1";
		d2.a = 42;
		REQUIRE(d1 == d2);
		d2.a = 43;
		REQUIRE(d1 != d2);

		std::unique_ptr<int> p1 = std::make_unique<int>(10);
		std::unique_ptr<int> p2 = std::make_unique<int>(10);
		auto e1 = p1 == p2;

		{
			xBase b1;
			b1.children.push_back(std::make_unique<xBase>("base1"));
			b1.children.push_back(std::make_unique<xBase>("base2"));

			xBase b2;
			b2.children.push_back(std::make_unique<xBase>("base1"));
			b2.children.push_back(std::make_unique<xBase>("base2"));
			auto b = b1 == b2;
			REQUIRE(b1 == b2);

			xBase b3;
			b3.children.push_back(std::make_unique<xBase>("base1"));
			b3.children.push_back(std::make_unique<xBase>("base3"));
			REQUIRE(b1 != b3);

			xBase b4;
			b4.children.push_back(std::make_unique<xBase>("base1"));
			auto r = std::make_unique<xDerived1>();
			r->name = "base2";
			b4.children.push_back(std::move(r));
			REQUIRE(b1 == b4);
		}

		static_assert( (1 <=> 0) > 0);
		static_assert(std::weak_ordering::less < 0);

	}


}

