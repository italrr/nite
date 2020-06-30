#ifndef NITE_TILESET_HPP
	#define NITE_TILESET_HPP
	#include "Graphics.hpp"
	#include "Texture.hpp"
	namespace nite {
		struct TileLayerSingle {
			nite::Vec2 inTextureCoors;
			nite::Vec2 inMapCoors;
			int tileIndex;
			int depth;
			int computedDepth;
			TileLayerSingle(){

			}
			TileLayerSingle(const nite::Vec2 &inTextureCoors, const nite::Vec2 &inMapCoors, int depth, int tileIndex){
				this->inTextureCoors.set(inTextureCoors);
				this->inMapCoors.set(inMapCoors);
				this->depth = depth;
				this->tileIndex = tileIndex;
			}
		};

		struct TileLayer {
			Vector<nite::TileLayerSingle> data;
			String name;
			nite::Vec2 position;
			int depth;
			bool depthTest;
			bool visible;
			bool ySort;
			bool floorDepth;
		};

		struct TileLayerBatchBuffer {
			// Vector<nite::TextureRegionBatch> batches;
			int depth;
		};

		class Tileset {
			public:
				nite::Vec2 lastViewPosition;
				nite::Batch batch;
				nite::Vec2 mapSize;
				nite::Vec2 tileMapSize;
				nite::Vec2 tileSize;
				nite::Vec2 imageSize;
				float spacing;
				float margin;
				String sourceFilename;
				nite::Texture source;
				Vector <nite::TileLayerBatchBuffer> tileBaches;
				nite::Renderable *draw(float x, float y);
				nite::Renderable *draw(float x, float y, float scaleX, float scaleY);
				nite::Renderable *draw(const nite::Vec2 &position);
				nite::Renderable *draw(const nite::Vec2 &position, const nite::Vec2 &scale);
				Tileset();
				Tileset(const String &path);
				~Tileset();
				void loadSources(const nite::Color &transparency);
				void load(const String &path);
				void clear();
			private:
				bool loaded;
		};
	}
#endif
