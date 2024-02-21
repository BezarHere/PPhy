#include <iostream>
#include <PPhy.h>

int main() {
	pphy::Space2D space{};
	pphy::Object2D object{ pphy::ObjectType::Static, pphy::ShapeType2D::None };
	space.add_object( object );
	space.update( 1. / 60. );
}
