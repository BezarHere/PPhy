#include "pch.h"
#include "plugphy.h"
using namespace pphy;

template Space2D;
template Space3D;
template Object2D;
template Object3D;

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


	template<typename _OBJ>
	TSpace<_OBJ>::TSpace() {
	}

	template<typename _OBJ>
	void TSpace<_OBJ>::update( real_t deltatime ) {
		m_batcher.try_rebuild( m_objects );
		const BatchResult &b = m_batcher.get_results();



	}

	template<typename _OBJ>
	void TSpace<_OBJ>::add_object( const object_type &object ) {
		m_objects.push_back( object );
		m_batcher.invalidate();
	}

	Shape2D::ShapeUnion2D::~ShapeUnion2D() {
	}

	Shape2D::ShapeUnion2D::ShapeUnion2D( const ShapeUnion2D &copy ) {
	}

	Shape3D::ShapeUnion3D::~ShapeUnion3D() {
	}

	Shape3D::ShapeUnion3D::ShapeUnion3D( const ShapeUnion3D &copy ) {
	}

	void Shape2D::recalculate_bounding_box() {
	}

	void Shape3D::recalculate_bounding_box() {
	}

}
