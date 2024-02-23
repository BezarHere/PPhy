#include "pch.h"
#include "pphy.h"
using namespace pphy;

constexpr real_t Epsilon =
#ifdef PPHY_HIPREC
1.0E-9;
#else
1.0E-4f;
#endif
;;;;;;;;;;;;

constexpr index_t PhysicsIterations = 10;

template Space2D;
template Space3D;
template Object2D;
template Object3D;

enum class CollisionType
{
	StaticStaticCollision = 0x0,
	StaticClipCollision = 0x1,
	StaticCharecterCollision = 0x2,
	StaticRigidCollision = 0x3,
	StaticSoftCollision = 0x4,
	ClipStaticCollision = 0x100,
	ClipClipCollision = 0x101,
	ClipCharecterCollision = 0x102,
	ClipRigidCollision = 0x103,
	ClipSoftCollision = 0x104,
	CharecterStaticCollision = 0x200,
	CharecterClipCollision = 0x201,
	CharecterCharecterCollision = 0x202,
	CharecterRigidCollision = 0x203,
	CharecterSoftCollision = 0x204,
	RigidStaticCollision = 0x300,
	RigidClipCollision = 0x301,
	RigidCharecterCollision = 0x302,
	RigidRigidCollision = 0x303,
	RigidSoftCollision = 0x304,
	SoftStaticCollision = 0x400,
	SoftClipCollision = 0x401,
	SoftCharecterCollision = 0x402,
	SoftRigidCollision = 0x403,
	SoftSoftCollision = 0x404
};

inline static constexpr CollisionType get_collision_type( const ObjectType type_a, const ObjectType type_b ) {
	return static_cast<CollisionType>(static_cast<int>(type_b) | (static_cast<int>(type_a) << 8));
}

inline static CollisionType get_collision_type( const Object2D &obj_a, const Object2D &obj_b ) {
	return get_collision_type( obj_a.get_type(), obj_b.get_type() );
}

inline static CollisionType get_collision_type( const Object3D &obj_a, const Object3D &obj_b ) {
	return get_collision_type( obj_a.get_type(), obj_b.get_type() );
}

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

				//if (obj.is_frame_dirty())
				//	obj.recalculate_frame();

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

#pragma region(Solvers: Tearing my hear out)
	namespace solvers
	{
		using I2D = Iterative2D;
		//using I3D = Iterative3D;



		static_assert(std::is_same_v<ShapeType2D, I2D::shape_type_enum>, "expects I2D::shape_type_enum to be the same as ShapeType2D");

		template<typename ShapeType2D ShapeTypeA, typename ShapeType2D ShapeTypeB>
		struct Iterative2DImpl
		{
			inline static bool process( I2D::object_ref_pair objects, I2D::shape_ref_pair shapes ) = delete;
		};

		template<>
		struct Iterative2DImpl<ShapeType2D::Circle, ShapeType2D::Circle>
		{
			inline static bool process( I2D::object_ref_pair objects, I2D::shape_ref_pair shapes ) {
				const real_t d =
					(shapes.first.get_circle().center + objects.first.get_position()).distance( shapes.second.get_circle().center + objects.second.get_position() );

				if (d >= shapes.first.get_circle().radius + shapes.second.get_circle().radius)
				{
					return false;
				}

				const Vector2 dir =
					(shapes.first.get_circle().center + objects.first.get_position()).direction( shapes.second.get_circle().center + objects.second.get_position() );
				const Vector2 push_amount = dir * (shapes.first.get_circle().radius + shapes.second.get_circle().radius - d);


				switch (get_collision_type( objects.first, objects.second ))
				{
				case CollisionType::StaticClipCollision:
				case CollisionType::CharecterClipCollision:
				case CollisionType::RigidClipCollision:
				case CollisionType::SoftClipCollision:
				case CollisionType::ClipClipCollision:
				case CollisionType::ClipStaticCollision:
				case CollisionType::ClipCharecterCollision:
				case CollisionType::ClipRigidCollision:
				case CollisionType::ClipSoftCollision:
					{
						// TODO: trigger clip
					}
					break;
				case CollisionType::RigidStaticCollision:
				case CollisionType::CharecterStaticCollision:
				case CollisionType::StaticRigidCollision:
				case CollisionType::StaticCharecterCollision:
					{
						if (objects.first.get_type() == ObjectType::Static)
						{
							// impulse object 2
							objects.second.set_position( objects.first.get_position() + push_amount );
						}
						else
						{
							// impulse object 1
							objects.first.set_position( objects.first.get_position() - push_amount );
						}
					}
					break;
				case CollisionType::RigidRigidCollision:
				case CollisionType::RigidCharecterCollision:
				case CollisionType::CharecterRigidCollision:
				case CollisionType::CharecterCharecterCollision:
					{
						if (objects.second.get_type() == objects.first.get_type())
						{
							objects.first.set_position( objects.first.get_position() - push_amount / static_cast<real_t>(2) );
							objects.second.set_position( objects.second.get_position() + push_amount / static_cast<real_t>(2) );
							break;
						}
					}
					break;
				default:
					break;
				}



				return true;
			}
		};

		template<>
		template<typename I2D::shape_type_enum ShapeTypeA, typename I2D::shape_type_enum ShapeTypeB>
		inline bool I2D::solve( object_ref_pair objects, shape_ref_pair shapes ) {
			return Iterative2DImpl<ShapeTypeA, ShapeTypeB>::process( objects, shapes );
		}
	}
#pragma endregion

	template<typename _VEC>
	void TPolygon<_VEC>::recalculate() {
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
	TObject<_STATE>::TObject( ObjectType type ) : m_type{ type }, m_shapes{}, m_frame{} {

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
		set_shape( shape, 0 );
	}

	template<typename _STATE>
	void TObject<_STATE>::set_shape( const shape_type &shape, index_t index ) {
		m_shapes[ index ] = shape;
		m_shapes[ index ].recalculate_bounding_box();
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::add_shape( const shape_type &shape ) {
		m_shapes.push_back( shape );
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::remove_shape( index_t index ) {
		m_shapes.erase( m_shapes.begin() + index );
		wakeup();
	}

	template<typename _STATE>
	void TObject<_STATE>::recalculate_frame() {
		m_frame_dirty = false;
		m_frame = {};
		for (shape_type &shape : m_shapes)
		{
			shape.recalculate_bounding_box();
			m_frame.encase( shape.get_bounding_box() );
		}
	}

	template<typename _OBJ>
	TSpace<_OBJ>::TSpace() : m_dt{} {
	}

	template<typename _OBJ>
	void TSpace<_OBJ>::update( real_t deltatime ) {
		if (deltatime - Epsilon <= 0)
			deltatime = Epsilon;

		m_dt = deltatime;

		/*
		- Do we need to recalculate the collision image (BatchResult) per iteration or per frame?
		+ currently it is per frame

		- Should each impulse be less effective?
		+ idk
		*/
		//constexpr solvers::I2D::SolverProc xx = solvers::I2D::solve<object_type::shape_type_enum::None, object_type::shape_type_enum::None>;

		// preprocessor
		for (object_type &obj : m_objects)
		{
			if (obj.is_frame_dirty())
				obj.recalculate_frame();
		}

		m_batcher.try_rebuild( m_objects );
		const BatchResult &batch_results = m_batcher.get_results();

		for (const ObjectBatch &batch : batch_results)
		{
			update( batch );
		}

	}

	template<typename _OBJ>
	void TSpace<_OBJ>::update( const ObjectBatch &objects ) {
		for (index_t iteration = 0; iteration < PhysicsIterations; iteration++)
		{

			for (size_t i = 0; i < objects.size(); i++)
			{
				object_type &object_a = m_objects[ objects[ i ] ];
				for (index_t j = i + 1; j < objects.size(); j++)
				{
					object_type &object_b = m_objects[ objects[ j ] ];

					if (object_a.get_type() == ObjectType::Static && object_b.get_type() == ObjectType::Static)
						continue;

					// bounding boxes not intersecting, objects can't be colliding
					if (!object_a.get_frame().intersects( object_b.get_frame() ))
						continue;


					//solvers::TIterative<object_type>::solve( { object_a, object_b }, { object_a.get_shape(), object_b.get_shape() } );
				}
			}

		}
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
			m_bounding_box = calculate_bounding_box( m_data.polygon );
			return;
		case ShapeType2D::Circle:
			m_bounding_box = calculate_bounding_box( m_data.circle );
			return;
		case ShapeType2D::Triangle:
			m_bounding_box = calculate_bounding_box( m_data.triangle );
			return;
		case ShapeType2D::Rectangle:
			m_bounding_box = calculate_bounding_box( m_data.rectangle );
			return;
		case ShapeType2D::Line:
			m_bounding_box = calculate_bounding_box( m_data.line );
			return;
		case ShapeType2D::Ray:
			m_bounding_box = calculate_bounding_box( m_data.ray );
			return;
		default:
			m_bounding_box = { 0, 0, 0, 0 };
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
