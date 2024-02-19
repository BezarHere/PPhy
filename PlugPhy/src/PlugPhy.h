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
		World, // <- backed static body
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
		vector_type::lower_rank base_size;
	};
	using Triangle = TPointy<Vector2>;
	using Pyramid = TPointy<Vector3>;

	struct Line
	{
		using vector_type = Vector2;

		vector_type origin;
		vector_type direction;
	};

	struct Plane
	{
		using vector_type = Vector3;

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


	private:
		shape_type_enum m_type;
		union ShapeUnion2D
		{
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


	private:
		shape_type_enum m_type;
		union ShapeUnion2D
		{
			Polygon3D polygon;
			Sphere sphere;
			Pyramid pyramid;
			Plane plane;
			Ray3D ray;
		} m_data;
	};

	struct ObjectState2D
	{
	public:
		using vector_type = Vector2;
		using shape_type = Shape2D;


	};

	struct ObjectState3D
	{
	public:
		using vector_type = Vector3;
		using shape_type = Shape3D;

	};

	template <typename _STATE>
	class TObject
	{
	public:
		using state_type = _STATE;
		using vector_type = state_type::vector_type;
		using this_type = TObject<_STATE>;


	private:
		ObjectType m_type;
		vector_type m_position;
		real_t m_angle;
		vector_type m_linear_velocity;
		vector_type m_angular_velocity;
		real_t m_mass;

		this_type *m_parent;
		std::vector<this_type *> m_children;

		state_type::shape_type m_shape;
		state_type m_state;
	};
	using Object2D = TObject<ObjectState2D>;
	using Object3D = TObject<ObjectState3D>;

	template <typename _OBJ>
	class TSpace
	{
	public:
		using object_type = _OBJ;

	private:
		const size_t m_space_size;
		std::unique_ptr<object_type[]> m_objects;
	};
	using Space2D = TSpace<Object2D>;
	using Space3D = TSpace<Object3D>;

	

}
