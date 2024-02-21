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

	// mathematically called 'pair of points'
	template <typename _T>
	using Segment = std::pair<_T, _T>;

	enum class ShapeType2D
	{
		Rectangle,
		Circle,
		Triangle,
		Line,
		Ray,
		Polygon,
	};

	enum class ShapeType3D
	{
		Box,
		Sphere,
		Pyramid,
		Plane,
		Ray,
		Polygon,
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
		ObjFlag_NeverSleeps = 0x0001,
	};
	using CollisionMask = uint32_t;

	template <typename _VEC>
	struct TFrame
	{
		using this_type = TFrame<_VEC>;
		using vector_type = _VEC;
		using value_type = typename vector_type::value_type;
		using lower_rank = TFrame<typename vector_type::lower_rank>;

		inline constexpr TFrame() : begin{}, end{} {
		}

		inline constexpr TFrame( const vector_type &begin_point, const vector_type &end_point )
			: begin{ begin_point }, end{ end_point } {
		}

		// only defined for TFrame<Vector3> (e.g. AABB)
		inline constexpr TFrame( value_type bx, value_type by, value_type bz, value_type ex, value_type ey, value_type ez );
		// only defined for TFrame<Vector2> (e.g. Rect)
		inline constexpr TFrame( value_type bx, value_type by, value_type ex, value_type ey );

		inline constexpr bool intersects( const this_type &other ) const;

		inline constexpr this_type expanded( value_type margin ) const;

		inline constexpr this_type encasing( const this_type &other ) const {
			this_type copy = *this;
			copy.encase( other );
			return copy;
		}

		inline constexpr void encase( const this_type &other );
		inline constexpr void encase( const vector_type &point );

		vector_type begin;
		vector_type end;
	};

	using Rect = TFrame<Vector2>;
	using AABB = TFrame<Vector3>;

	template <typename _VEC>
	class TPolygon
	{
	public:
		using vector_type = _VEC;
		using frame_type = TFrame<vector_type>;

		inline vector_type get_center() const {
			return m_center;
		}

		inline Winding get_winding() const {
			return m_winding;
		}

		inline const frame_type &get_bounds() const {
			return m_bounds;
		}

		inline void try_update() {
			if (m_dirty)
				update();
		}
		inline void update();

	private:
		bool m_dirty;
		vector_type m_center;
		frame_type m_bounds;
		Winding m_winding;
		std::vector<vector_type> m_points;
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

	// mathematically called 'ray'
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

	// mathematically called 'segment'
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

		inline const Rect &get_rectangle() const noexcept {
			return m_data.rectangle;
		}

		inline Rect &get_rectangle() noexcept {
			return m_data.rectangle;
		}

		inline const Polygon2D &get_polygon() const noexcept {
			return m_data.polygon;
		}

		inline Polygon2D &get_polygon() noexcept {
			return m_data.polygon;
		}

		inline const Circle &get_circle() const noexcept {
			return m_data.circle;
		}

		inline Circle &get_circle() noexcept {
			return m_data.circle;
		}

		inline const Triangle &get_triangle() const noexcept {
			return m_data.triangle;
		}

		inline Triangle &get_triangle() noexcept {
			return m_data.triangle;
		}

		inline const Line &get_line() const noexcept {
			return m_data.line;
		}

		inline Line &get_line() noexcept {
			return m_data.line;
		}

		inline const Ray2D &get_ray() const noexcept {
			return m_data.ray;
		}

		inline Ray2D &get_ray() noexcept {
			return m_data.ray;
		}

		inline Rect get_bounding_rect() const noexcept {
			return m_bounding_rect;
		}

		void recalculate_bounding_box();

	private:
		shape_type_enum m_type;
		Rect m_bounding_rect;
		union ShapeUnion2D
		{
			ShapeUnion2D( const ShapeUnion2D &copy );
			ShapeUnion2D &operator=( const ShapeUnion2D &other );
			~ShapeUnion2D();

			Polygon2D polygon;
			Circle circle;
			Triangle triangle;
			Line line;
			Ray2D ray;
			Rect rectangle;
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

		void recalculate_bounding_box();

	private:
		shape_type_enum m_type;
		AABB m_aabb;
		union ShapeUnion3D
		{
			ShapeUnion3D( const ShapeUnion3D &copy );
			ShapeUnion3D &operator=( const ShapeUnion3D &other );
			~ShapeUnion3D();

			Polygon3D polygon;
			Sphere sphere;
			Pyramid pyramid;
			Plane plane;
			Ray3D ray;
			AABB box;
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
		using shape_type = typename state_type::shape_type;

		inline frame_type get_frame() const;

		inline ObjectType get_type() const {
			return m_type;
		}

		inline ObjectFlags get_flags() const {
			return m_flags;
		}

		inline const vector_type &get_position() const {
			return m_position;
		}

		inline real_t get_angle() const {
			return m_angle;
		}

		inline real_t get_angular_velocity() const {
			return m_angular_velocity;
		}

		inline const vector_type &get_linear_velocity() const {
			return m_position;
		}

		inline real_t get_mass() const {
			return m_mass;
		}

		void set_position( const vector_type &value );
		void set_angle( real_t value );
		void set_angular_velocity( real_t value );
		void set_linear_velocity( const vector_type &value );

		inline void activate() {
			wakeup();
			m_active = true;
		}

		inline void deactivate() {
			m_active = false;
		}

		inline bool is_activated() const {
			return m_active;
		}

		inline CollisionMask get_mask() const {
			return m_mask;
		}

		void set_mask( CollisionMask mask );

		inline void wakeup() {
			m_awake = true;
		}

		inline bool is_awake() const {
			return m_awake;
		}

		inline bool is_always_awake() const {
			return m_flags & ObjFlag_NeverSleeps;
		}

		inline const shape_type &get_shape() const {
			return m_shape;
		}

		void set_shape( const shape_type &shape );

	private:
		ObjectType m_type;
		ObjectFlags m_flags;
		bool m_awake;
		bool m_active;
		vector_type m_position;
		real_t m_angle;
		vector_type m_linear_velocity;
		real_t m_angular_velocity;
		real_t m_mass;
		CollisionMask m_mask;

		shape_type m_shape;
		//state_type m_state; // <- might remove later if not needed
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

		/// @brief adds the object to the object list
		/// @note will invalidate batcher which in turn is pretty expensive
		void add_object( const object_type &object );
		inline const object_type &get_object( index_t index ) const {
			return m_objects[ index ];
		}

		inline object_type &get_object( index_t index ) {
			return m_objects[ index ];
		}

	private:
		batcher_type m_batcher;
		std::vector<object_type> m_objects;
	};
	using Space2D = TSpace<Object2D>;
	using Space3D = TSpace<Object3D>;

#pragma region(Definitions)

	template<>
	inline constexpr TFrame<Vector3>::TFrame( value_type bx, value_type by, value_type bz, value_type ex, value_type ey, value_type ez )
		: begin{ bx, by, bz }, end{ ex, ey, ez } {
	}

	template<>
	inline constexpr TFrame<Vector2>::TFrame( value_type bx, value_type by, value_type ex, value_type ey )
		: begin{ bx, by }, end{ ex, ey } {
	}

	template<>
	inline constexpr bool TFrame<Vector2>::intersects( const this_type &other ) const {
		return other.end.x < begin.x || other.end.y < begin.y || other.begin.x > end.x || other.begin.y > end.y;
	}

	template<>
	inline constexpr bool TFrame<Vector3>::intersects( const this_type &other ) const {
		return other.end.x < begin.x || other.end.y < begin.y || other.end.z < begin.z || other.begin.x > end.x || other.begin.y > end.y || other.begin.z > end.z;
	}

	template<>
	inline constexpr TFrame<Vector2>::this_type TFrame<Vector2>::expanded( value_type margin ) const {
		return { begin.x - margin, begin.y - margin , end.x + margin, end.y + margin };
	}

	template<>
	inline constexpr TFrame<Vector3>::this_type TFrame<Vector3>::expanded( value_type margin ) const {
		return { begin.x - margin, begin.y - margin, begin.z - margin, end.x + margin, end.y + margin, end.z + margin };
	}

	template<>
	inline constexpr void TFrame<Vector2>::encase( const this_type &other ) {
		begin.x = std::min( begin.x, other.begin.x );
		begin.y = std::min( begin.y, other.begin.y );
		end.x = std::max( end.x, other.end.x );
		end.y = std::max( end.y, other.end.y );
	}

	template<>
	inline constexpr void TFrame<Vector3>::encase( const this_type &other ) {
		begin.x = std::min( begin.x, other.begin.x );
		begin.y = std::min( begin.y, other.begin.y );
		begin.z = std::min( begin.z, other.begin.z );
		end.x = std::max( end.x, other.end.x );
		end.y = std::max( end.y, other.end.y );
		end.z = std::max( end.z, other.end.z );
	}

	template<>
	inline constexpr void TFrame<Vector2>::encase( const vector_type &other ) {
		begin.x = std::min( begin.x, other.x );
		begin.y = std::min( begin.y, other.y );
		end.x = std::max( end.x, other.x );
		end.y = std::max( end.y, other.y );
	}

	template<>
	inline constexpr void TFrame<Vector3>::encase( const vector_type &other ) {
		begin.x = std::min( begin.x, other.x );
		begin.y = std::min( begin.y, other.y );
		begin.z = std::min( begin.z, other.z );
		end.x = std::max( end.x, other.x );
		end.y = std::max( end.y, other.y );
		end.z = std::max( end.z, other.z );
	}

	template <typename _VEC>
	inline constexpr bool TRound<_VEC>::is_point_inside( const vector_type &point ) const {
		return (point - center).length_squared() <= radius;
	}

	template<typename _VEC>
	inline constexpr bool TRound<_VEC>::is_intersecting_segment( const vector_type &p0, const vector_type &p1, segment_type &intersection ) const {
		(void)p0;
		(void)p1;
		(void)intersection;
		return false;
	}

	inline constexpr bool Line::is_point_front( const Vector2 &vec ) const {
		return direction.dot( vec - origin ) > 0;
	}

	inline constexpr bool Line::is_intersecting_segment( const Vector2 &p0, const Vector2 &p1, Vector2 &intersection ) const {
		(void)p0;
		(void)p1;
		(void)intersection;
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


#pragma endregion
}
