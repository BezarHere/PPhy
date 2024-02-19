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


	enum class ObjectType2D
	{
		Circle,
		Triangle,
		Rectangle,
		Line,
		Ray,
	};

	enum class ObjectType3D
	{
		Sphere,
		Box,
		Plane,
		Ray
	};

	enum class Winding
	{
		// use: when the polygon is empty, it is not clockwise nor counter clockwise
		None,
		Clockwise,
		CounterClockwise
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

	struct ObjectState2D
	{
	public:
		using vector_type = Vector2;


	};

	struct ObjectState3D
	{
	public:
		using vector_type = Vector3;

	};

	template <typename _STATE>
	class TObject
	{
	public:
		using state_type = _STATE;
		using vector_type = state_type::vector_type;
		using this_type = TObject<_STATE>;


	private:
		vector_type m_position;
		real_t m_angle;
		vector_type m_linear_velocity;
		vector_type m_angular_velocity;
		real_t m_mass;

		this_type *m_parent;
		std::vector<this_type *> m_children;

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
