#include "pch.h"
#include "pphy.h"
using namespace pphy;

template Space2D;
template Space3D;
template Object2D;
template Object3D;

inline static Rect calculate_bounding_box( const Polygon2D &polygon ) {
	return polygon.get_bounds();
}

inline static Rect calculate_bounding_box( const Circle &circle ) {
	return {
		circle.center.x - circle.radius, circle.center.y - circle.radius,
		circle.center.x + circle.radius, circle.center.y + circle.radius
	};
}

inline static Rect calculate_bounding_box( const Rect &rect ) {
	return rect;
}

inline static Rect calculate_bounding_box( const Triangle &triangle ) {
	const Vector2 base_extent = triangle.head_extent.normalized().tangent();
	return {
		std::min( { base_extent.x, -base_extent.x, triangle.head_extent.x } ) + triangle.base.x,
		std::min( { base_extent.y, -base_extent.y, triangle.head_extent.y } ) + triangle.base.y,
		std::max( { base_extent.x, -base_extent.x, triangle.head_extent.x } ) + triangle.base.x,
		std::max( { base_extent.y, -base_extent.y, triangle.head_extent.y } ) + triangle.base.y
	};
}

inline static Rect calculate_bounding_box( const Line &line ) {
	return {
		line.direction.x < 0 ? -HUGE_VALF : line.origin.x,
		line.direction.y < 0 ? -HUGE_VALF : line.origin.y,
		line.direction.x > 0 ? HUGE_VALF : line.origin.x,
		line.direction.y > 0 ? HUGE_VALF : line.origin.y
	};
}

inline static Rect calculate_bounding_box( const Ray2D &ray ) {
	return {
		std::min<real_t>( 0, ray.extent.x ) + ray.origin.x,
		std::min<real_t>( 0, ray.extent.y ) + ray.origin.y,
		std::max<real_t>( 0, ray.extent.x ) + ray.origin.x,
		std::max<real_t>( 0, ray.extent.y ) + ray.origin.y
	};
}

// author: ??? (heavily modified by me)
inline static bool is_clockwise( const Vector2 *const points, const size_t count ) {
	real_t turn_factor{};

	//if (count < 3)
	//	return true;

	for (size_t i{}; i < count - 1; i++)
	{
		turn_factor += (points[ i ].x - points[ i + 1 ].x) * (points[ i ].y + points[ i + 1 ].y);
	}

	return turn_factor > -(points[ count - 1 ].x - points[ 0 ].x) * (points[ count - 1 ].y + points[ 0 ].y);
}

inline static bool is_clockwise( const Vector3 *const points, const size_t count ) {
	(void)count;
	(void)points;
	// FIXME: how should this be calculated
	return false;
}

namespace pphy
{
	namespace batchers
	{
		static constexpr size_t BatchResultInitCapacity = 1u << 8;
		static constexpr size_t ObjectBatchInitCapacity = 1u << 8;

		template<typename _OBJ>
		TBoundsBatcher<_OBJ>::TBoundsBatcher() : m_results{} {
			m_results.reserve( BatchResultInitCapacity );
			for (ObjectBatch &batch : m_results)
			{
				batch.reserve( ObjectBatchInitCapacity );
			}
		}

		template<typename _OBJ>
		void TBoundsBatcher<_OBJ>::invalidate() {
			m_dirty = true;
		}

		template<typename _OBJ>
		void TBoundsBatcher<_OBJ>::rebuild( const std::vector<object_type> &objects ) {
			m_dirty = false;

			m_results.clear();

			frame_type *const frames = new frame_type[ objects.size() ];
			frame_type *const group_frames = new frame_type[ objects.size() ];

			for (index_t i = 0; i < objects.size(); i++)
			{
				const object_type &obj = objects[ i ];

				// no need
				if (!obj.is_activated())
					continue;

				//?? msvc++ be like 'BufFeR OveRruN wHilE WRiTinG tO 'frames'.'
				frames[ i ] = obj.get_frame().expanded( m_expand_margin );

				bool found_group = false;
				// single object's aabb can intersect multiple group frames
				for (index_t group_index = 0; group_index < m_results.size(); group_index++)
				{
					if (frames[ i ].intersects( group_frames[ group_index ] ))
					{
						group_frames[ group_index ].encase( frames[ i ] );
						m_results[ group_index ].push_back( i );
						found_group = true;
					}
				}

				if (!found_group)
				{
					m_results.emplace_back();
					m_results.back().push_back( i );
					group_frames[ m_results.size() - 1 ] = frames[ i ];
				}

			}

			delete[] frames;
			delete[] group_frames;
		}
	}

	template<typename _VEC>
	void TPolygon<_VEC>::update() {
		m_dirty = false;
		if (m_points.size() < 3)
		{
			m_winding = Winding::None;

		}
		else
		{
			m_winding =
				is_clockwise( m_points.data(), m_points.size() ) ? Winding::Clockwise : Winding::CounterClockwise;
		}

		m_center = {};

		if (m_points.empty())
			return;

		m_bounds = { m_points[ 0 ], m_points[ 0 ] };


		for (index_t i = 0; i < m_points.size(); i++)
		{
			m_center += m_points[ i ];
			m_bounds.encase( m_points[ i ] );
		}
		m_center /= static_cast<real_t>(m_points.size());

	}

	template<typename _STATE>
	TObject<_STATE>::TObject( ObjectType type, shape_type_enum shape_type ) : m_type{ type }, m_shape{ shape_type } {

	}

	template<typename _STATE>
	void TObject<_STATE>::set_position( const vector_type &value ) {
		m_position = value;
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::set_angle( real_t value ) {
		m_angle = value;
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::set_angular_velocity( real_t value ) {
		m_angular_velocity = value;
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::set_linear_velocity( const vector_type &value ) {
		m_linear_velocity = value;
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::set_mask( const CollisionMask mask ) {
		m_mask = mask;
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::set_shape( const shape_type &shape ) {
		m_shape = shape;
		m_shape.recalculate_bounding_box();
		wakeup();
	}

	template<typename _OBJ>
	TSpace<_OBJ>::TSpace() {
	}

	template<typename _OBJ>
	void TSpace<_OBJ>::update( real_t deltatime ) {
		(void)deltatime;

		m_batcher.try_rebuild( m_objects );
		const BatchResult &batch_results = m_batcher.get_results();
		(void)batch_results;

		// do the solving...

	}

	template<typename _OBJ>
	void TSpace<_OBJ>::add_object( const object_type &object ) {
		m_objects.push_back( object );
		m_batcher.invalidate();
	}

#pragma region(ShapeUnion)

	// FIXME: why the polygon??
	Shape2D::ShapeUnion2D::ShapeUnion2D( shape_type_enum type ) : polygon{} {
		(void)type;
		// FIXME: use 'type'
	}

	Shape2D::ShapeUnion2D::ShapeUnion2D( const ShapeUnion2D &copy ) : polygon{ copy.polygon } {
	}

	Shape2D::ShapeUnion2D::~ShapeUnion2D() {
	}

	Shape2D::ShapeUnion2D &Shape2D::ShapeUnion2D::operator=( const ShapeUnion2D &other ) {
		// FIXME: why the polygon??
		polygon = other.polygon;
		return *this;
	}

	Shape3D::ShapeUnion3D::ShapeUnion3D( shape_type_enum type ) : polygon{} {
		(void)type;
		// FIXME: use 'type'
	}

	// FIXME: why the polygon??
	Shape3D::ShapeUnion3D::ShapeUnion3D( const ShapeUnion3D &copy ) : polygon{ copy.polygon } {
	}

	Shape3D::ShapeUnion3D::~ShapeUnion3D() {
	}

	Shape3D::ShapeUnion3D &Shape3D::ShapeUnion3D::operator=( const ShapeUnion3D &other ) {
		// FIXME: why the polygon??
		polygon = other.polygon;
		return *this;
	}
#pragma endregion

	Shape2D::Shape2D() : m_data{ ShapeType2D::Rectangle } {
	}

	Shape2D::Shape2D( shape_type_enum type ) : m_data{ type } {
	}

	void Shape2D::recalculate_bounding_box() {
		switch (m_type)
		{
		case ShapeType2D::Polygon:
			m_bounding_rect = calculate_bounding_box( m_data.polygon );
			return;
		case ShapeType2D::Circle:
			m_bounding_rect = calculate_bounding_box( m_data.circle );
			return;
		case ShapeType2D::Triangle:
			m_bounding_rect = calculate_bounding_box( m_data.triangle );
			return;
		case ShapeType2D::Rectangle:
			m_bounding_rect = calculate_bounding_box( m_data.rectangle );
			return;
		case ShapeType2D::Line:
			m_bounding_rect = calculate_bounding_box( m_data.line );
			return;
		case ShapeType2D::Ray:
			m_bounding_rect = calculate_bounding_box( m_data.ray );
			return;
		default:
			m_bounding_rect = { 0, 0, 0, 0 };
			break;
		}
	}

	Shape3D::Shape3D() : m_data{ ShapeType3D::Box } {
	}

	Shape3D::Shape3D( shape_type_enum type ) : m_data{ type } {
	}

	void Shape3D::recalculate_bounding_box() {
	}

}
