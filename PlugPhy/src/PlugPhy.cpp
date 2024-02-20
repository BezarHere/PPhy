#include "pch.h"
#include "plugphy.h"
using namespace pphy;

template Space2D;
template Space3D;
template Object2D;
template Object3D;

namespace pphy
{
	template<typename _OBJ>
	TSpace<_OBJ>::TSpace()
	{
	}

	Shape2D::ShapeUnion2D::~ShapeUnion2D() {
	}

	Shape3D::ShapeUnion3D::~ShapeUnion3D() {
	}




}
