#ifndef NITE_SHAPE_HPP
	#define NITE_SHAPE_HPP
	#include "Graphics.hpp"
	namespace nite {
		struct RenderableRectangle : public nite::Renderable {
			nite::Vec2 size;
			nite::Vec2 origin;
			float angle;
			bool fill;
			float thickness;
		};
		struct RenderableLine : public nite::Renderable {
			float thickness;
			nite::Vec2 positionStart;	
			nite::Vec2 positionEnd;			
		};
		struct RenderablePolygon : public nite::Renderable {
			float thickness;
			bool fill;		
			nite::Vec2 origin;
			nite::Polygon shape;
		};		
		namespace Draw {
			nite::RenderableRectangle *Rectangle(float x, float y, float w, float h, bool fill, float origX, float origY, float angle);
			nite::RenderableRectangle *Rectangle(float x, float y, float w, float h, bool fill);
			nite::RenderableRectangle *Rectangle(const nite::Vec2 &P, const nite::Vec2 &S, bool fill, const nite::Vec2 &orig, float angle);
			nite::RenderableRectangle *Rectangle(const nite::Vec2 &P, const nite::Vec2 &S, bool fill);
			nite::RenderableLine *Line(float x, float y, float x2, float y2);
			nite::RenderablePolygon *Polygon(const nite::Polygon &polygon, const nite::Vec2 &pos, const nite::Vec2 &origin, bool fill);
		}
	}
#endif
