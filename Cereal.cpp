#include <catch2/catch_all.hpp>

#include <iostream>


// Include the polymorphic serialization and registration mechanisms
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/polymorphic.hpp>

#include <iostream>
#include <fstream>

// A pure virtual base class
struct BaseClass {
	virtual void sayType() = 0;
};

// A class derived from BaseClass
struct DerivedClassOne : public BaseClass {
	void sayType();

	int x;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(x);
	}
};

// Another class derived from BaseClass
struct EmbarrassingDerivedClass : public BaseClass {
	void sayType();

	float y;

	template<class Archive>
	void serialize(Archive& ar) {
		ar(y);
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

// Register EmbarassingDerivedClass with a less embarrasing name
CEREAL_REGISTER_TYPE_WITH_NAME(EmbarrassingDerivedClass, "DerivedClassTwo");

// Note that there is no need to register the base class, only derived classes
//  However, since we did not use cereal::base_class, we need to clarify
//  the relationship (more on this later)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, DerivedClassOne)
CEREAL_REGISTER_POLYMORPHIC_RELATION(BaseClass, EmbarrassingDerivedClass)






void DerivedClassOne::sayType() {
	std::cout << "DerivedClassOne" << std::endl;
}

void EmbarrassingDerivedClass::sayType() {
	std::cout << "EmbarrassingDerivedClass. Wait.. I mean DerivedClassTwo!" << std::endl;
}



TEST_CASE("Cereal", "[Cereal]") {
	{
		std::ofstream os("polymorphism_test.xml");
		cereal::XMLOutputArchive oarchive(os);

		// Create instances of the derived classes, but only keep base class pointers
		std::shared_ptr<BaseClass> ptr1 = std::make_shared<DerivedClassOne>();
		std::shared_ptr<BaseClass> ptr2 = std::make_shared<EmbarrassingDerivedClass>();
		oarchive(ptr1, ptr2);
	}

	{
		std::ifstream is("polymorphism_test.xml");
		cereal::XMLInputArchive iarchive(is);

		// De-serialize the data as base class pointers, and watch as they are
		// re-instantiated as derived classes
		std::shared_ptr<BaseClass> ptr1;
		std::shared_ptr<BaseClass> ptr2;
		iarchive(ptr1, ptr2);

		// Ta-da! This should output:
		ptr1->sayType();  // "DerivedClassOne"
		ptr2->sayType();  // "EmbarrassingDerivedClass. Wait.. I mean DerivedClassTwo!"
	}

}
