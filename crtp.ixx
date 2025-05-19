module;

#include <catch.hpp>
export module crtp;
import std;

namespace {

	struct sBase {
	public:
		int a{};
	public:

		auto operator <=> (sBase const&) const = default;
	};

	template < typename tderived, typename tvalue >
	struct TBase : public sBase {
	public:
		using derived_t = tderived;
		using value_t = tvalue;

	public:
		value_t m_value{};

	public:
		virtual ~TBase() = default;
		auto operator <=> (TBase const&) const = default;

		value_t& operator () () {
		}
	};

	struct sObjectA : public TBase<sObjectA, int> {
	};

	struct sObjectB : public TBase<sObjectB, int> {
	};

	static_assert(std::is_same_v<sObjectA::value_t, int>);

	TEST_CASE("derived", "crtp") {


		sObjectA a, a2;
		sObjectB b, b2;


		REQUIRE(a == a2);
		REQUIRE(b == b2);
	}

}
