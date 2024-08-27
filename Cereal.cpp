#include <catch2/catch_all.hpp>

// Include the polymorphic serialization and registration mechanisms
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/polymorphic.hpp>

#include "fmt/core.h"

import std.compat;

// A pure virtual base class
struct BaseClass {
	virtual void sayType() = 0;
};

template < typename T >
void Func(T&& t) {
	fmt::print("{}", t);
	if constexpr (std::is_const_v<std::remove_reference_t<decltype(t)>>) {
		fmt::print(" is const\n");
	}
	else {
		fmt::print(" is not const\n");
		t += "sdfsdf";
	}

	if constexpr (std::is_reference_v<decltype(t)>) {
		fmt::print(" is reference\n");
	}
	else {
		fmt::print(" is not reference\n");
	}
}

// A class derived from BaseClass
struct DerivedClassOne : public BaseClass {
	DerivedClassOne() : x(0) {}
	DerivedClassOne(int x) : x(x) {}
	void sayType();

	int x;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(x);
	}
};

// Another class derived from BaseClass
struct EmbarrassingDerivedClass : public BaseClass {
	EmbarrassingDerivedClass() : BaseClass(), y{} {}
	EmbarrassingDerivedClass(float y) : BaseClass(), y(y) {}

	void sayType();

	float y;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(y);
	}
};

struct DerivedClassA : public DerivedClassOne {
	using base_t = DerivedClassOne;
	using this_t = DerivedClassA;

	base_t& base() { return *this; }
	base_t const& base() const { return *this; }

	constexpr static uint32_t s_version = 1;
	DerivedClassA() : DerivedClassOne(), y{} {}
	DerivedClassA(int x, float y) : DerivedClassOne(x), y(y) {}
	void sayType();
	float y;

	template<class Archive>
	void serialize(Archive& ar, std::uint32_t version) {
		constexpr static auto sl = std::source_location::current();
		if constexpr (Archive::is_loading()) {
			fmt::println("{} loading...", sl.function_name());
		}
		else {
			fmt::println("{} storing...", sl.function_name());
		}
		ar(base(), y);
	}
};

// Include any archives you plan on using with your type before you register it
// Note that this could be done in any other location so long as it was prior
// to this file being included
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>

// Register DerivedClassOne
CEREAL_REGISTER_TYPE(DerivedClassOne);
CEREAL_REGISTER_TYPE(DerivedClassA)

// Register EmbarassingDerivedClass with a less embarrasing name
CEREAL_REGISTER_TYPE_WITH_NAME(EmbarrassingDerivedClass, "DerivedClassTwo");

// Note that there is no need to register the base class, only derived classes
//  However, since we did not use cereal::base_class, we need to clarify
//  the relationship (more on this later)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, DerivedClassOne)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, EmbarrassingDerivedClass)
CEREAL_REGISTER_POLYMORPHIC_RELATION(DerivedClassOne, DerivedClassA)
static_assert(std::is_base_of_v<DerivedClassOne, DerivedClassA>);

CEREAL_CLASS_VERSION(DerivedClassA, DerivedClassA::s_version);




void DerivedClassOne::sayType() {
	fmt::println("DerivedClassOne {}", x);
}

void DerivedClassA::sayType() {
	fmt::println("DerivedClassA {} {}", x, y);
}

void EmbarrassingDerivedClass::sayType() {
	fmt::println("EmbarrassingDerivedClass. Wait.. I mean DerivedClassTwo! {} ", y);
}

template <typename T, typename TPTR>
T* GetAs(TPTR& ptr) {
	return dynamic_cast<T*>(ptr.get());
}
template <typename T, typename TPTR>
T const* GetAs(TPTR const& ptr) {
	return dynamic_cast<T const*>(ptr.get());
}

TEST_CASE("Cereal", "[Cereal]") {
	{
		std::string const str = "Hello, World!";
		std::string const& str2 = str;
		Func(str2);

		// Create instances of the derived classes, but only keep base class pointers
		std::shared_ptr<BaseClass> ptr1 = std::make_shared<DerivedClassOne>(1);
		std::shared_ptr<BaseClass> ptr2 = std::make_shared<EmbarrassingDerivedClass>(2);
		std::shared_ptr<BaseClass> ptrA = std::make_shared<DerivedClassA>(5, 6.0f);
		try {
			{
				std::ofstream os("polymorphism_test.xml");
				cereal::XMLOutputArchive oarchive(os);
				oarchive(ptr1, ptr2, ptrA);
			}
			{
				std::ofstream os("polymorphism_test.json");
				cereal::JSONOutputArchive oarchive(os);
				oarchive(ptr1, ptr2, ptrA);
			}
			{
				std::ofstream os("polymorphism_test.bin");
				cereal::BinaryOutputArchive oarchive(os);
				oarchive(ptr1, ptr2, ptrA);
			}
		}
		catch (std::exception& e) {
			fmt::print("{}", e.what());
		}
		catch (...) {
			fmt::print("Unknown exception");
		}
	}

	{
		std::ifstream is("polymorphism_test.xml");
		cereal::XMLInputArchive iarchive(is);

		// De-serialize the data as base class pointers, and watch as they are
		// re-instantiated as derived classes
		std::shared_ptr<BaseClass> ptr1;
		std::shared_ptr<BaseClass> ptr2;
		std::shared_ptr<BaseClass> ptrA;
		{
			iarchive(ptr1, ptr2, ptrA);
			// Ta-da! This should output:
			ptr1->sayType();  // "DerivedClassOne"
			ptr2->sayType();  // "EmbarrassingDerivedClass. Wait.. I mean DerivedClassTwo!"
			ptrA->sayType();  // "DerivedClassA"
			REQUIRE(GetAs<DerivedClassOne>(ptr1)->x == 1);
			REQUIRE(GetAs<EmbarrassingDerivedClass>(ptr2)->y == 2);
			REQUIRE(GetAs<DerivedClassA>(ptrA)->x == 5);
			REQUIRE(GetAs<DerivedClassA>(ptrA)->y == 6.0f);
		}
	}

	{
		std::ifstream is("polymorphism_test.json");
		cereal::JSONInputArchive iarchive(is);

		// De-serialize the data as base class pointers, and watch as they are
		// re-instantiated as derived classes
		std::shared_ptr<BaseClass> ptr1;
		std::shared_ptr<BaseClass> ptr2;
		std::shared_ptr<BaseClass> ptrA;
		{
			iarchive(ptr1, ptr2, ptrA);
			// Ta-da! This should output:
			ptr1->sayType();  // "DerivedClassOne"
			ptr2->sayType();  // "EmbarrassingDerivedClass. Wait.. I mean DerivedClassTwo!"
			ptrA->sayType();  // "DerivedClassA"
			REQUIRE(GetAs<DerivedClassOne>(ptr1)->x == 1);
			REQUIRE(GetAs<EmbarrassingDerivedClass>(ptr2)->y == 2);
			REQUIRE(GetAs<DerivedClassA>(ptrA)->x == 5);
			REQUIRE(GetAs<DerivedClassA>(ptrA)->y == 6.0f);
		}
	}

	{
		std::ifstream is("polymorphism_test.bin");
		cereal::BinaryInputArchive iarchive(is);

		// De-serialize the data as base class pointers, and watch as they are
		// re-instantiated as derived classes
		std::shared_ptr<BaseClass> ptr1;
		std::shared_ptr<BaseClass> ptr2;
		std::shared_ptr<BaseClass> ptrA;
		{
			iarchive(ptr1, ptr2, ptrA);
			// Ta-da! This should output:
			ptr1->sayType();  // "DerivedClassOne"
			ptr2->sayType();  // "EmbarrassingDerivedClass. Wait.. I mean DerivedClassTwo!"
			ptrA->sayType();  // "DerivedClassA"
			REQUIRE(GetAs<DerivedClassOne>(ptr1)->x == 1);
			REQUIRE(GetAs<EmbarrassingDerivedClass>(ptr2)->y == 2);
			REQUIRE(GetAs<DerivedClassA>(ptrA)->x == 5);
			REQUIRE(GetAs<DerivedClassA>(ptrA)->y == 6.0f);
		}
	}
}
