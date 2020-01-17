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

		struct TextureRegionBatch {
			nite::Vec2 position;
			nite::Vec2 size;
			int depth;
			Vector<nite::TextureRegionSingle> regions;
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
			bool repeat; // otherwise stretch. stretch  is by default
			nite::TextureRegionBatch *batch;
			RenderableTextureT(){
				repeat = true;
			}
		};

		struct RenderableTextureBatchT : public nite::Renderable {
			int objectId;
			bool smooth;
			nite::TextureRegionBatch *batch;
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
				nite::RenderableTextureBatchT *draw(nite::TextureRegionBatch *batch, float x, float y);
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
				float getWidth();
				float getHeight();
				Texture& operator= (const Texture &other);
				void setScale(float x, float y);
				void setScale(const nite::Vec2 &S);
				int getTextureId();
				void unload();
			private:
				nite::Vec2 scale;
				bool smooth;
				nite::Rect region;
				int objectId;
		};
	}
#endif
