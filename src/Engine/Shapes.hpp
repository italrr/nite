#ifndef NITE_SHAPE
	#define NITE_SHAPE
	#include "Graphics.hpp"
	namespace nite {
		struct RenderableRectangle : public nite::Renderable {
			nite::Vec2 size;
			nite::Vec2 origin;
			float angle;
			bool fill;
		};
		struct RenderableLine : public nite::Renderable {
			nite::Vec2 positionStart;	
			nite::Vec2 positionEnd;			
		};
		namespace Draw {
			nite::RenderableRectangle *Rectangle(float x, float y, float w, float h, bool fill, float origX, float origY, float angle);
			nite::RenderableRectangle *Rectangle(float x, float y, float w, float h, bool fill);
			nite::RenderableRectangle *Rectangle(const nite::Vec2 &P, const nite::Vec2 &S, bool fill, const nite::Vec2 &orig, float angle);
			nite::RenderableRectangle *Rectangle(const nite::Vec2 &P, const nite::Vec2 &S, bool fill);

			nite::RenderableLine *Line(float x, float y, float x2, float y2);
		}
	}
#endif
