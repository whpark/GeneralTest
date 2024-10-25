#include <catch2/catch_all.hpp>

// Include the polymorphic serialization and registration mechanisms
#include <cereal/archives/xml.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>

#include "fmt/core.h"

import std;
import cereal_test;

//// Register DerivedClassOne
//CEREAL_REGISTER_TYPE(ct::DerivedClassOne);
//CEREAL_REGISTER_TYPE(ct::DerivedClassA)
//
//// Register EmbarassingDerivedClass with a less embarrasing name
//CEREAL_REGISTER_TYPE_WITH_NAME(ct::EmbarrassingDerivedClass, "DerivedClassTwo");
//
//// Note that there is no need to register the base class, only derived classes
////  However, since we did not use cereal::base_class, we need to clarify
////  the relationship (more on this later)
//CEREAL_REGISTER_POLYMORPHIC_RELATION(ct::BaseClass, ct::DerivedClassOne)
//CEREAL_REGISTER_POLYMORPHIC_RELATION(ct::BaseClass, ct::EmbarrassingDerivedClass)
//CEREAL_REGISTER_POLYMORPHIC_RELATION(ct::DerivedClassOne, ct::DerivedClassA)
//
//CEREAL_CLASS_VERSION(ct::DerivedClassA, ct::DerivedClassA::s_version);


namespace ct {


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
			std::stringstream ss;
			{
				std::vector<int> lst { 1, 2, 3, 4, 5 };
				cereal::BinaryOutputArchive ar(ss);
				ar(cereal::BinaryData(lst.data(), lst.size()*sizeof(lst.data()[0])));
			}
			{
				std::vector<int> lst;
				lst.resize(5);
				cereal::BinaryInputArchive ar(ss);
				ar(cereal::BinaryData(lst.data(), lst.size()*sizeof(lst.data()[0])));
				REQUIRE(lst[4] == 5);
			}
		}
		{
			// Create instances of the derived classes, but only keep base class pointers
			std::shared_ptr<BaseClass> ptr1 = std::make_shared<DerivedClassOne>(1);
			std::shared_ptr<BaseClass> ptr2 = std::make_shared<EmbarrassingDerivedClass>(2.0f);
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

}	// namespace ct
