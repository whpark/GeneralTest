module;

#include <catch2/catch_all.hpp>

// Include the polymorphic serialization and registration mechanisms
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

#include <fmt/core.h>
#include <opencv2/opencv.hpp>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

export module cereal_test;
import std;

// for cereal serialization of cv::Mat
namespace detail {
	// https://stackoverflow.com/questions/7053538/how-do-i-encode-a-string-to-base64-using-only-boost
	std::vector<uint8_t> decode64(std::string const& val) {
		using namespace boost::archive::iterators;
		using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
		return boost::algorithm::trim_right_copy_if(std::vector<uint8_t>(It(std::begin(val)), It(std::end(val))), [](char c) {
			return c == '\0';
		});
	}

	std::string encode64(std::span<uint8_t const> val) {
		using namespace boost::archive::iterators;
		using It = base64_from_binary<transform_width<std::span<uint8_t const>::const_iterator, 6, 8>>;
		auto tmp = std::string(It(val.begin()), It(val.end()));
		return tmp.append((3 - val.size() % 3) % 3, '=');
	}
}
export namespace cv {
	template <class tArchive>
	void save(tArchive& ar, cv::Mat const& mat, uint32_t const version) {
		constexpr bool bTEXT_ARCHIVE = cereal::traits::is_text_archive<tArchive>::value;
		bool bContinuous = !bTEXT_ARCHIVE and mat.isContinuous();
		ar( cereal::make_nvp("rows", mat.rows),
			cereal::make_nvp("cols", mat.cols),
			cereal::make_nvp("type", mat.type()),
			cereal::make_nvp("continuous", bContinuous));

		if (mat.empty())
			return;

		if constexpr (bTEXT_ARCHIVE) {
			size_t const step = mat.cols * mat.elemSize();
			for (int i = 0; i < mat.rows; i++) {
				ar & ::detail::encode64(std::span{mat.ptr(i), step});
			}
		}
		else {
			if (mat.isContinuous()) {
				ar& cereal::binary_data(mat.ptr(), (size_t)mat.step * mat.rows);
			}
			else {
				size_t const step = mat.cols * mat.elemSize();
				for (int i = 0; i < mat.rows; i++)
					ar & cereal::binary_data(mat.ptr(i), step);
			}
		}
	}

	template <class tArchive>
	void load(tArchive& ar, cv::Mat& mat, uint32_t const version) {
		constexpr bool bTEXT_ARCHIVE = cereal::traits::is_text_archive<tArchive>::value;
		int rows{}, cols{}, type{};
		bool bContinuous{};

		ar( cereal::make_nvp("rows", rows),
			cereal::make_nvp("cols", cols),
			cereal::make_nvp("type", type),
			cereal::make_nvp("continuous", bContinuous));

		if (rows <= 0 or cols <= 0 or type <= 0) {
			mat.release();
			return;
		}

		if (mat.rows != rows or mat.cols != cols or mat.type() != type)
			mat = cv::Mat::zeros(rows, cols, type);

		if constexpr (bTEXT_ARCHIVE) {
			size_t const step = cols * mat.elemSize();
			for (int i = 0; i < rows; i++) {
				std::string encoded;
				ar & encoded;
				auto decoded = ::detail::decode64(encoded);
				memcpy(mat.ptr(i), decoded.data(), step);
			}
		}
		else {
			if (bContinuous) {
				ar & cereal::binary_data(mat.ptr(), (size_t)mat.step * mat.rows);
			}
			else {
				size_t const step = cols * mat.elemSize();
				for (int i = 0; i < rows; i++) {
					ar & cereal::binary_data(mat.ptr(i), step);
				}
			}
		}
	}
}

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
		std::u8string extra_data;

		template<class Archive>
		void serialize(Archive& ar, std::uint32_t const version) {
			constexpr static auto sl = std::source_location::current();
			if constexpr (Archive::is_loading()) {
				fmt::println("{} loading...", sl.function_name());
			}
			else {
				fmt::println("{} storing...", sl.function_name());
			}
			ar(base(), y, (std::string&)extra_data);
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


export {
// Register DerivedClassOne
CEREAL_REGISTER_TYPE(ct::BaseClass);
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
}

