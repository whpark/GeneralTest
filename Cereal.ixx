module;

#include <catch2/catch_all.hpp>

// Include the polymorphic serialization and registration mechanisms
#include <cereal/archives/xml.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>

#include "fmt/core.h"

export module cereal_test;
import std;

export namespace ct {

	// A pure virtual base class
	struct BaseClass {
		virtual void sayType() = 0;
	};

	// A class derived from BaseClass
	struct DerivedClassOne : public BaseClass {
		DerivedClassOne() : x(0) {}
		DerivedClassOne(int x) : x(x) {}
		void sayType();

		int x;

		template<class Archive>
		void serialize(Archive& ar) {
			ar(cereal::make_nvp("X", x));
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
		void serialize(Archive& ar, std::uint32_t const version) {
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
	static_assert(std::is_base_of_v<DerivedClassOne, DerivedClassA>);

	// Include any archives you plan on using with your type before you register it
	// Note that this could be done in any other location so long as it was prior
	// to this file being included

	//// Register DerivedClassOne
	//CEREAL_REGISTER_TYPE(DerivedClassOne);
	//CEREAL_REGISTER_TYPE(DerivedClassA)

	//// Register EmbarassingDerivedClass with a less embarrasing name
	//CEREAL_REGISTER_TYPE_WITH_NAME(EmbarrassingDerivedClass, "DerivedClassTwo");

	//// Note that there is no need to register the base class, only derived classes
	////  However, since we did not use cereal::base_class, we need to clarify
	////  the relationship (more on this later)
	//CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, DerivedClassOne)
	//CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, EmbarrassingDerivedClass)
	//CEREAL_REGISTER_POLYMORPHIC_RELATION(DerivedClassOne, DerivedClassA)
	//static_assert(std::is_base_of_v<DerivedClassOne, DerivedClassA>);

	//CEREAL_CLASS_VERSION(DerivedClassA, DerivedClassA::s_version);


	void DerivedClassOne::sayType() {
		fmt::println("DerivedClassOne {}", x);
	}

	void DerivedClassA::sayType() {
		fmt::println("DerivedClassA {} {}", x, y);
	}

	void EmbarrassingDerivedClass::sayType() {
		fmt::println("EmbarrassingDerivedClass. Wait.. I mean DerivedClassTwo! {} ", y);
	}

}


// Register DerivedClassOne
CEREAL_REGISTER_TYPE(ct::DerivedClassOne);
CEREAL_REGISTER_TYPE(ct::DerivedClassA)

// Register EmbarassingDerivedClass with a less embarrasing name
CEREAL_REGISTER_TYPE_WITH_NAME(ct::EmbarrassingDerivedClass, "DerivedClassTwo");

// Note that there is no need to register the base class, only derived classes
//  However, since we did not use cereal::base_class, we need to clarify
//  the relationship (more on this later)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ct::BaseClass, ct::DerivedClassOne)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ct::BaseClass, ct::EmbarrassingDerivedClass)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ct::DerivedClassOne, ct::DerivedClassA)

CEREAL_CLASS_VERSION(ct::DerivedClassA, ct::DerivedClassA::s_version);
