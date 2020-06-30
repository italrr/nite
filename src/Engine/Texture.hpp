#ifndef NITE_TEXTURE_HPP
	#define NITE_TEXTURE_HPP
	#include "Graphics.hpp"
	namespace nite {

		struct TextureRegionSingle {
			float box[8];
			float texBox[8];
			nite::Vec2 inTextureCoors;
			nite::Vec2 inDrawCoors;
		};

		struct TextureCellBatch {
			nite::Vec2 size; // in pixels
			int depth;
			char *cells;
			int total;
			int w, h;
			bool setSize(int w, int h, float wp, float hp); // in units!
			void clear();
			bool add(int index, nite::TextureRegionSingle *src);
			TextureCellBatch();
			~TextureCellBatch();
		};

		struct RenderableTextureT : public nite::Renderable {
			nite::Vec2 tsize;
			nite::Vec2 size;
			nite::Vec2 origin;
			nite::Rect region;
			float angle;
			int objectId;
			bool smooth;
			nite::Vec2 scale;
			bool repeat; // otherwise stretch. stretch is by default
			RenderableTextureT(){
				repeat = true;
			}
		};

		struct RenderableTextureBatchT : public nite::Renderable {
			int x, y, vpx, vpy, vpw, vph;
			int objectId;
			bool smooth;
			nite::TextureCellBatch *batch;
		};

		class Texture {
			public:
				bool isLoaded();
				bool load(const String &path);
				bool load(const String &path, const nite::Color &transparency);
				Texture();
				Texture(const String &path, const nite::Color &transparency);
				Texture(const String &path);
				Texture(const nite::Texture &other);
				~Texture();
				nite::RenderableTextureBatchT *drawCellBatch(nite::TextureCellBatch *batch, float x, float y, float vpx, float vpy, float vpw, float vph);
				nite::RenderableTextureT *draw(float x, float y);
				nite::RenderableTextureT *draw(float x, float y, float angle);
				nite::RenderableTextureT *draw(const nite::Vec2 &P);
				nite::RenderableTextureT *draw(const nite::Vec2 &P, float angle);
				nite::RenderableTextureT *draw(const nite::Vec2 &P, const nite::Vec2 &S, const nite::Vec2 &orig, float angle);
				nite::RenderableTextureT *draw(float x, float y, float w, float h, float origX, float origY, float angle);
				void setRegion(float x, float y, float w, float h);
				void setRegion(const nite::Vec2 &P, const nite::Vec2 &S);
				void setRegion(const nite::Rect &R);
				void setSmooth(bool S);
				void setStick(bool v);
				Vec2 getSize();
				String getFilename();
				float getWidth();
				float getHeight();
				Texture& operator= (const Texture &other);
				void setScale(float x, float y);
				void setScale(const nite::Vec2 &S);
				int getTextureId();
				void unload();
			private:
				String filename;
				nite::Vec2 scale;
				bool smooth;
				nite::Rect region;
				int objectId;
		};
	}
#endif
