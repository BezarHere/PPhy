#include <iostream>
#include <PPhy.h>

int main() {
	pphy::Space2D space{};
	pphy::Object2D object{ pphy::ObjectType::Static };
	space.add_object( object );
	space.update( 1. / 60. );
}
