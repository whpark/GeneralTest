module;

#include <catch.hpp>
#include <eigen3/Eigen/Dense>

export module eigen;
import std;
using namespace std::literals;

namespace test {

	TEST_CASE("alignedbox", "eigen") {
		Eigen::AlignedBox3d box{Eigen::Vector3d{ 0, 0, 0 }, Eigen::Vector3d{ 1, 1, 1 }};
		box.extend(Eigen::Vector3d{2, 2, 2});
		REQUIRE(box.max() == Eigen::Vector3d{2, 2, 2});

		box.setEmpty();
		box.extend(Eigen::Vector3d{-1, 1, 0});
		REQUIRE(box.min() == Eigen::Vector3d{-1, 1, 0});
		REQUIRE(box.max() == Eigen::Vector3d{-1, 1, 0});

		box.setNull();
		box.extend(Eigen::Vector3d{-1, 1, 0});
		REQUIRE(box.min() == Eigen::Vector3d{-1, 1, 0});
		REQUIRE(box.max() == Eigen::Vector3d{-1, 1, 0});

		box.min() = Eigen::Vector3d{-2, 0, -1};
		box.max() = Eigen::Vector3d{0, 2, 1};

		Eigen::Vector3d pt{0, 1, 0};
		REQUIRE(pt[0] == 0);
		REQUIRE(box.contains(pt));

		Eigen::Vector3d pt2{1, 1, 0};
		auto pt3 = pt.cross(pt2);
		REQUIRE(pt3 == Eigen::Vector3d{0., 0.0, -1.0});

		pt3.normalize();
	}

	template < int nDim, Eigen::TransformTraits eMode = Eigen::TransformTraits::Affine, int eOption = 0 >
	class TCoordTrans {
	public:
		static_assert(nDim == 2 or nDim == 3);
		static_assert(eMode == Eigen::TransformTraits::Isometry or eMode == Eigen::TransformTraits::Affine or eMode == Eigen::TransformTraits::Projective);
	public:
		using this_t = TCoordTrans;
		using point_t = Eigen::Vector<double, nDim>;
		using transform_t = Eigen::Transform<double, nDim, eMode>;
		inline static constexpr int dim = nDim;
		inline static constexpr Eigen::TransformTraits mode = eMode;

	public:
		point_t m_origin{};
		transform_t m_transform{transform_t::Identity()};

	public:
		auto operator <=> (TCoordTrans const& other) const {
			if (auto cmp = m_origin <=> other.m_origin; cmp != 0) return cmp;
			return m_transform.matrix().array() <=> other.m_transform.matrix().array();
		}
		bool operator == (TCoordTrans const& other) const {
			return m_origin == other.m_origin && m_transform.matrix() == other.m_transform.matrix();
		}

		//template < int nDim2, Eigen::TransformTraits eMode2, int eOption2 >
		//void SetFrom(TCoordTrans<nDim2, eMode2, eOption2> const& other) {
		//	static_assert(nDim >= nDim2, "Cannot set from a higher dimension transform");
		//	//static_assert(eMode == eMode2, "Cannot set from a different transform mode");
		//	m_origin = other.m_origin;
		//	m_transform = other.m_transform;
		//}

		point_t operator() (point_t const& pt) const {
			return m_transform * (pt - m_origin);
		}

		point_t const& GetOffset() const { return m_transform.translation(); }
		void SetOffset(point_t const& offset) { m_transform.translation() = offset; }
		double CalcScale() const {
			auto det = m_transform.linear().determinant();
			return std::pow(std::abs(det), 1.0 / nDim);
		}
		double SetScale(double scale) {
			auto currentScale = CalcScale();
			if (currentScale == 0) {
				m_transform.linear().setIdentity();
				return 0.0;
			}
			double factor = scale / currentScale;
			m_transform.linear() *= factor;
			return currentScale;
		}
		bool IsRightHanded() const {
			return m_transform.linear().determinant() > 0;
		}
		point_t Translation() const {
			return m_transform(-m_origin);
		}
		transform_t TransformMatrix() const {
			transform_t shift;
			shift.translation() = -m_origin;
			return m_transform * shift;
		}

		std::optional<TCoordTrans> Inverse() const {
			TCoordTrans inverse;
			inverse.m_transform = m_transform;
			inverse.m_transform.translation().setZero();
			if (inverse.m_transform.matrix().determinant() == 0.0) {
				return std::nullopt;
			}
			inverse.m_transform = inverse.m_transform.inverse((Eigen::TransformTraits)m_transform.Mode);
			inverse.m_origin = m_transform.translation();
			inverse.m_transform.translation() = m_origin;
			return inverse;
		}
	};

	using xCoordTrans2d = TCoordTrans<2, Eigen::TransformTraits::Affine>;
	using xCoordTrans3d = TCoordTrans<3, Eigen::TransformTraits::Affine>;
	using xCoordTrans2dP = TCoordTrans<2, Eigen::TransformTraits::Projective>;
	using xCoordTrans3dP = TCoordTrans<3, Eigen::TransformTraits::Projective>;

	TEST_CASE("coordtrans", "eigen") {
		xCoordTrans2d ct2d;
		ct2d.m_origin = {10., 20.};
		ct2d.SetOffset({1, 2});
		ct2d.SetScale(2.0);
		auto ct2d_copy = ct2d;
		REQUIRE(ct2d == ct2d_copy);
		auto ct2d_inverse = ct2d.Inverse();
		REQUIRE(ct2d_inverse.has_value());
		REQUIRE(ct2d_inverse->IsRightHanded() == ct2d.IsRightHanded());
		auto pt = Eigen::Vector2d{4, 5};
		auto pt2 = ct2d(pt);
		auto pt3 = (*ct2d_inverse)(pt2);
		REQUIRE((pt - pt3).norm() < 1e-6);
	}

}

