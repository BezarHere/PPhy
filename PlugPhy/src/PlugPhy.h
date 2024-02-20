#pragma once
/*
PlugPhy Physics Library
Feel free to implement in your product given you abide to the license


zahr abdullatif babker (C) 2024-2025
*/

#include "pphy/base.h"
#include "pphy/vector.h"
#include <vector>

namespace pphy
{
	typedef size_t index_t;
	typedef size_t hash_t;
	template <typename _T>
	using Segment = std::pair<_T, _T>;

	enum class ShapeType2D
	{
		Circle,
		Triangle,
		Rectangle,
		Line,
		Ray,
	};

	enum class ShapeType3D
	{
		Sphere,
		Pyramid,
		Box,
		Plane,
		Ray
	};

	enum class ObjectType
	{
		World, // <- baked static body
		Terrain = World,
		Static,
		Charecter,
		Rigid,
		Soft
	};

	enum class Winding
	{
		Clockwise,
		CounterClockwise,
		// use: when the polygon is empty, it is not clockwise nor counter clockwise
		None = 0xffff,
	};

	enum ObjectFlags : uint16_t
	{
		ObjFlag_None = 0x0000,
		ObjFlag_Clip = 0x0001,
	};
	using CollisionMask = uint32_t;

	template <typename _VEC>
	struct TFrame
	{
		using this_type = TFrame<_VEC>;
		using vector_type = _VEC;
		using value_type = typename vector_type::value_type;
		using lower_rank = TFrame<typename vector_type::lower_rank>;

		inline constexpr this_type intersects( const this_type &other ) const {
			return other.end.x < begin.x || other.end.y < begin.y || other.begin.x > end.x || other.begin.y > end.y;
		}

		inline constexpr this_type expanded( value_type margin ) const {
			return { begin.x - margin, begin.y - margin, end.x + margin, end.y + margin };
		}

		inline constexpr this_type encasing( const this_type &other ) const {
			this_type copy = *this;
			copy.encase( other );
			return copy;
		}

		inline constexpr this_type encase( const this_type &other ) {
			begin.x = std::min( begin.x, other.begin.x );
			begin.y = std::min( begin.y, other.begin.y );
			end.x = std::max( end.x, other.end.x );
			end.y = std::max( end.y, other.end.y );
		}

		vector_type begin;
		vector_type end;
	};

	using Rect = TFrame<Vector2>;
	using AABB = TFrame<Vector3>;

	template <typename _VEC>
	struct TPolygon
	{
	public:
		using vector_type = _VEC;

		inline vector_type get_center() const {
			return m_center;
		}

		inline Winding get_winding() const {
			return m_cw;
		}

	private:
		vector_type m_center;
		std::vector<vector_type> m_points;
		Winding m_cw;
	};
	using Polygon2D = TPolygon<Vector2>;
	using Polygon3D = TPolygon<Vector3>;
	using Polygon4D = TPolygon<Vector4>;

	template <typename _VEC>
	struct TRound
	{
		using vector_type = _VEC;
		using segment_type = Segment<vector_type>;

		inline constexpr bool is_point_inside( const vector_type &point ) const;
		inline constexpr bool is_intersecting_segment( const vector_type &p0, const vector_type &p1, segment_type &intersection ) const;

		vector_type center;
		float radius;
	};
	using Circle = TRound<Vector2>;
	using Sphere = TRound<Vector3>;
	//using HyperSphere = TRound<Vector4>;

	template <typename _VEC>
	struct TPointy
	{
		using vector_type = _VEC;

		vector_type base;
		vector_type head_extent;
		typename vector_type::lower_rank base_size;
	};
	using Triangle = TPointy<Vector2>;
	using Pyramid = TPointy<Vector3>;

	struct Line
	{
		using vector_type = Vector2;

		inline constexpr bool is_point_front( const Vector2 &vec ) const;
		inline constexpr bool is_intersecting_segment( const Vector2 &p0, const Vector2 &p1, Vector2 &intersection ) const;

		vector_type origin;
		vector_type direction;
	};

	struct Plane
	{
		using vector_type = Vector3;

		inline constexpr bool is_point_front( const Vector3 &vec ) const;
		inline constexpr bool is_intersecting_line( const Vector3 &p0, const Vector3 &p1 ) const;

		vector_type origin;
		vector_type normal;
		vector_type::lower_rank size;
	};

	template <typename _T>
	struct TRay
	{
		using vector_type = _T;

		vector_type origin;
		vector_type extent;
	};
	using Ray2D = TRay<Vector2>;
	using Ray3D = TRay<Vector3>;
	using Ray4D = TRay<Vector4>;

	struct Shape2D
	{
	public:
		using vector_type = Vector2;
		using shape_type_enum = ShapeType2D;

		inline Rect get_bounding_rect() const noexcept {
			return m_bounding_rect;
		}

	private:
		shape_type_enum m_type;
		Rect m_bounding_rect;
		union ShapeUnion2D
		{
			~ShapeUnion2D();

			Polygon2D polygon;
			Circle circle;
			Triangle triangle;
			Line line;
			Ray2D ray;
		} m_data;
	};

	struct Shape3D
	{
	public:
		using vector_type = Vector3;
		using shape_type_enum = ShapeType3D;

		inline AABB get_aabb() const noexcept {
			return m_aabb;
		}

	private:
		shape_type_enum m_type;
		AABB m_aabb;
		union ShapeUnion3D
		{
			~ShapeUnion3D();

			Polygon3D polygon;
			Sphere sphere;
			Pyramid pyramid;
			Plane plane;
			Ray3D ray;
		} m_data;
	};

	struct ObjectState2D
	{
		using vector_type = Vector2;
		using shape_type = Shape2D;
	};

	struct ObjectState3D
	{
		using vector_type = Vector3;
		using shape_type = Shape3D;
	};

	template <typename _STATE>
	class TObject
	{
	public:
		using state_type = _STATE;
		using vector_type = typename state_type::vector_type;
		using frame_type = typename TFrame<vector_type>;
		using this_type = TObject<_STATE>;

		inline frame_type get_frame() const;

	private:
		ObjectType m_type;
		ObjectFlags m_flags;
		vector_type m_position;
		real_t m_angle;
		vector_type m_linear_velocity;
		vector_type m_angular_velocity;
		real_t m_mass;
		CollisionMask m_mask;

		index_t m_parent;
		std::vector<index_t> m_children;

		typename state_type::shape_type m_shape;
		state_type m_state; // <- check if this is actually needed
	};
	using Object2D = TObject<ObjectState2D>;
	using Object3D = TObject<ObjectState3D>;

	using ObjectBatch = std::vector<index_t>;
	using BatchResult = std::vector<ObjectBatch>;

	namespace batchers
	{

		template <typename _OBJ>
		class TBoundsBatcher
		{
		public:
			using object_type = _OBJ;
			using frame_type = typename object_type::frame_type;
			TBoundsBatcher();

			inline const BatchResult &get_results() const {
				return m_results;
			}

			void invalidate();

			inline void try_rebuild( const std::vector<object_type> &objects ) {
				if (m_dirty)
					rebuild( objects );
			}

			void rebuild( const std::vector<object_type> &objects );

		private:
			bool m_dirty = true;
			BatchResult m_results;
			real_t m_expand_margin = 0.25f;
		};
		using BoundsBatcher2D = TBoundsBatcher<Object2D>;
		using BoundsBatcher3D = TBoundsBatcher<Object3D>;

	}

	template <typename _OBJ>
	class TSpace
	{
	public:
		using object_type = _OBJ;
		using batcher_type = batchers::TBoundsBatcher<object_type>;
		friend batcher_type;

		TSpace();

		void update( real_t deltatime );

	private:
		batcher_type m_batcher;
		std::vector<object_type> m_objects;
	};
	using Space2D = TSpace<Object2D>;
	using Space3D = TSpace<Object3D>;


	template <typename _VEC>
	inline constexpr bool TRound<_VEC>::is_point_inside( const vector_type &point ) const {
		return (point - center).length_squared() <= radius;
	}

	template<typename _VEC>
	inline constexpr bool TRound<_VEC>::is_intersecting_segment( const vector_type &p0, const vector_type &p1, segment_type &intersection ) const {
		return false;
	}

	inline constexpr bool Line::is_point_front( const Vector2 &vec ) const {
		return direction.dot( vec - origin ) > 0;
	}

	inline constexpr bool Line::is_intersecting_segment( const Vector2 &p0, const Vector2 &p1, Vector2 &intersection ) const {
		return false;
	}

	inline constexpr bool Plane::is_point_front( const Vector3 &vec ) const {
		return normal.dot( vec - origin ) > 0;
	}

	inline constexpr bool Plane::is_intersecting_line( const Vector3 &p0, const Vector3 &p1 ) const {
		(void)p0;
		(void)p1;
		return false;
	}

	template<>
	inline TObject<ObjectState2D>::frame_type TObject<ObjectState2D>::get_frame() const {
		return m_shape.get_bounding_rect();
	}

	template<>
	inline TObject<ObjectState3D>::frame_type TObject<ObjectState3D>::get_frame() const {
		return m_shape.get_aabb();
	}

}
