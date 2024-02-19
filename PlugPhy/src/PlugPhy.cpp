#include "pch.h"
#include "plugphy.h"

namespace pphy
{
	template<typename _OBJ>
	TSpace<_OBJ>::TSpace( const size_t objcount )
		: m_space_size{ objcount }, m_objects{ new object_type[ objcount ] } {
	}

	Shape2D::ShapeUnion2D::~ShapeUnion2D() {
	}

	Shape3D::ShapeUnion3D::~ShapeUnion3D() {
	}
}
