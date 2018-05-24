#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "SOIL/SOIL.h"
#include "View.hpp"
#include "Tools/Tools.hpp"
#include "Texture.hpp"
#include "Tileset.hpp"

nite::Tileset::Tileset(){
  loaded = false;
  lastViewPosition.set(0.0);
}

nite::Tileset::Tileset(const String &path){
  loaded = false;
  load(path);
}

nite::Tileset::~Tileset(){
  clear();
}

struct DepthMask {
  nite::Vec2 position;
  nite::Vec2 absSize;
  nite::Vec2 size;
  float origin;
  String depthScope;
  nite::Vec2 mOrigin;
  DepthMask(const nite::Vec2 &position, const nite::Vec2 &absSize, float origin, const String &name){
    this->position.set(position);
    this->absSize.set(absSize);
    this->origin = origin;
    this->size.set(this->absSize - this->position);
    this->mOrigin.set(this->size * nite::Vec2(this->origin));
    this->depthScope = name;
  }
  bool isInRegion(const nite::Vec2 &p){
    return isPointInRegion(p, position, absSize);
  }
  float correct(nite::Vec2 &v){
    return (position.y + mOrigin.y) - v.y;
  }
};

void nite::Tileset::loadSources(const nite::Color &transparency){
  source.load(sourceFilename, transparency);
}

struct TileBatchCompiled {
  int depth;
  Vector<nite::TextureRegionSingle> compiled;
};

static bool isDepthInArray(int depth, Vector<int> &depths){
  for(int c = 0; c < depths.size(); ++c){
    if(depths[c] == depth){
      return true;
    }
  }
  return false;
}

void nite::Tileset::load(const String &path){
  Jzon::Parser parser;
  Jzon::Node node = parser.parseFile(path);
  if (!node.isValid()){
		 nite::print("Failed to read file "+path+": '"+parser.getError()+"'");
	}
	UInt64 startTime = nite::getTicks();
  // Basic data
  mapSize.set(node.get("width").toInt(), node.get("height").toInt());
  tileSize.set(node.get("tilewidth").toInt(), node.get("tileheight").toInt());
  //transparency nite::Color

  // Read first tileset
  Jzon::Node tilesets = node.get("tilesets").get(0);
  sourceFilename = "data/map/"+tilesets.get("image").toString();
  imageSize.set(tilesets.get("imagewidth").toFloat(), tilesets.get("imageheight").toFloat());
  spacing = tilesets.get("spacing").toFloat();
  margin = tilesets.get("margin").toFloat();
  tileMapSize.set(nite::round(imageSize / (tileSize + spacing)));

  // Read depth_masks
  Jzon::Node layers = node.get("layers");

  Vector<DepthMask> masks;
  for(int i = 0; i < layers.getCount(); ++i){
    if(layers.get(i).get("type").toString() != "objectgroup") continue;
    Jzon::Node objs = layers.get(i).get("objects");
    for(int c = 0; c < objs.getCount(); ++c){
      if(objs.get(c).get("type").toString() != "depth_mask") continue;
      nite::Vec2 position = nite::Vec2(objs.get(c).get("x").toFloat(), objs.get(c).get("y").toFloat());
      nite::Vec2 absSize = nite::Vec2(objs.get(c).get("width").toFloat(), objs.get(c).get("height").toFloat());
      float depthOffset = objs.get(c).get("properties").get("depth_offset").toFloat();
      String depthScope = objs.get(c).get("properties").get("depth_layer").toString();
      masks.push_back(DepthMask(position, position + absSize, depthOffset, depthScope));
    }
  }

  auto checkBlock = [&](const nite::Vec2 &v, const String &name){
    for(auto vi : masks){
      if(vi.isInRegion(v) && vi.depthScope == name){
        return true;
      }
    }
    return false;
  };

  auto getMask = [&](const nite::Vec2 &v, const String &name){
    for(auto vi : masks){
      if(vi.isInRegion(v) && vi.depthScope == name){
        return vi;
      }
    }
    return DepthMask(nite::Vec2(0.0f), nite::Vec2(0.0f), 0.0f, "");
  };
  Vector <nite::TileLayer> _layers;
  bool isThereYSorts = false;
  // Read Layers
  for(int i = 0; i < layers.getCount(); ++i){
    if(layers.get(i).get("type").toString() != "tilelayer") continue;
    nite::TileLayer layer;
    layer.position = nite::Vec2(layers.get(i).get("x").toFloat(), layers.get(i).get("y").toFloat());
    layer.depth = layers.get(i).get("properties").get("depth").toInt();
    layer.depthTest = layers.get(i).get("properties").get("depth_test").toBool();
    layer.name = layers.get(i).get("name").toString();
    layer.visible = layers.get(i).get("visible").toBool();
    layer.ySort  = layers.get(i).get("properties").get("y_sort").toBool();
    layer.floorDepth  = layers.get(i).get("properties").get("floor_depth").toBool();
    if(layer.ySort) isThereYSorts = true;
    Jzon::Node data = layers.get(i).get("data");
    for(int c = 0; c < data.getCount(); ++c){
      float p = data.get(c).toInt();
      if(p == 0) continue;
      float tileY = nite::floor((p-1) / (this->tileMapSize.x));
      float tileX = (p-1) - tileY * (this->tileMapSize.x);
      float inY = nite::floor(c / mapSize.x);
      float inX = c - inY * mapSize.x;
      int depth = 0;
      nite::Vec2 inTextureCoors;
      nite::Vec2 inMapCoors;
      inTextureCoors.x = tileX * (tileSize.x + spacing) + margin;
      inTextureCoors.y = tileY * (tileSize.y + spacing) + margin;
      inMapCoors.x = inX * tileSize.x;
      inMapCoors.y = inY * tileSize.y;
      if(checkBlock(inMapCoors, layer.name)){
        DepthMask currentMask = getMask(inMapCoors, layer.name);
        depth = currentMask.correct(inMapCoors);
      }
      layer.data.push_back(TileLayerSingle(inTextureCoors, inMapCoors, depth, (int)p));
    }
    _layers.push_back(layer);
  }

  // merge 'y_order' layers and sort all tiles using 'y'
  if(isThereYSorts){
    // create merged
    nite::TileLayer merged;
    merged.position = nite::Vec2(0.0);
    merged.depth = 0;
    merged.depthTest = false;
    merged.name = "y_sorted"; // there's only one y_sorted per map
    merged.visible = false;
    merged.ySort  = false;
    merged.floorDepth = false;
    for(int i = 0; i < _layers.size(); ++i){
      if(!_layers[i].ySort) continue;
      nite::TileLayer &ref = _layers[i];
      merged.position = merged.position + ref.position;
      merged.position = merged.depth + ref.depth;
      if(ref.visible){
        merged.visible = true;
      }
      if(ref.depthTest){
        merged.depthTest = true;
      }
      if(ref.floorDepth){
        merged.floorDepth = true;
      }
      for(int c = 0; c < ref.data.size(); ++c){
        TileLayerSingle &tls = ref.data[c];
        if(checkBlock(tls.inMapCoors, merged.name)){
          DepthMask currentMask = getMask(tls.inMapCoors, merged.name);
          tls.depth = currentMask.correct(tls.inMapCoors);
        }
        merged.data.push_back(tls);
      }
      // remove originals
      _layers.erase(_layers.begin() + i);
      i = -1;
    }
    merged.position = merged.position / _layers.size();
    merged.depth = merged.depth / _layers.size();

    // sort it (possible violation of strict-weak-ordering)
    // TODO: Fix this, Italo  ^ lol
    std::sort(merged.data.begin(), merged.data.end(), [] (const nite::TileLayerSingle &a, const nite::TileLayerSingle &b){
      return a.inMapCoors.y < b.inMapCoors.y || (a.inMapCoors.y == b.inMapCoors.y && a.tileIndex > b.tileIndex);
    });

    // add it
    _layers.push_back(merged);
  }

  // precomputed depth
  for(int i = 0; i < _layers.size(); ++i){
    nite::TileLayer &current = _layers[i];
    for(int c = 0; c < current.data.size() ; ++c){
      nite::TileLayerSingle &tile = current.data[c];
      int depth = -(0.0f + tile.inMapCoors.y) - (current.depthTest ? tile.depth : current.depth);
      if(current.floorDepth){
        depth = nite::DepthBottom;
      }
      tile.computedDepth = depth;
    }
  }

  auto split = [&](nite::TileLayer &layer){
    Vector<TileBatchCompiled> compiled;
    // get depths
    Vector<int> depths;
    for(int i = 0; i < layer.data.size(); ++i){
      if(!isDepthInArray(layer.data[i].computedDepth, depths)){
        depths.push_back(layer.data[i].computedDepth);
      }
    }
    std::sort(depths.begin(), depths.end());
    // compile similar depths;
    for(int i = 0; i < depths.size(); ++i){
      int depth = depths[i];
      TileBatchCompiled tcb;
      tcb.depth = depth;
      for(int i = 0; i < layer.data.size(); ++i){
        if(depth != layer.data[i].computedDepth) continue;
        nite::TextureRegionSingle single;
        nite::TileLayerSingle &orig = layer.data[i];
        single.inTextureCoors.set(orig.inTextureCoors);
        single.inDrawCoors.set(orig.inMapCoors);
		int tw = imageSize.x; // Texture width
		int th = imageSize.y; // Texture height
		float itx = orig.inTextureCoors.x; // inTextureCoors x
		float ity = orig.inTextureCoors.y; // inTextureCoors y
		float bfsx = tileSize.x; // batch single size width
		float bfsy = tileSize.y; // batch single size height
		float dx = orig.inMapCoors.x; // in draw coors px
		float dy = orig.inMapCoors.y; // in draw coors py
        single.box[0] = dx;
        single.box[1] = dy;
        single.box[2] = dx + bfsx;
        single.box[3] = dy;
        single.box[4] = dx + bfsx;
        single.box[5] = dy + bfsy;
        single.box[6] = dx;
        single.box[7] = dy + bfsy;
        single.texBox[0] = itx / tw;
        single.texBox[1] = ity / th;
        single.texBox[2] = (itx + bfsx) / tw;
        single.texBox[3] = ity / th;
        single.texBox[4] =(itx + bfsx) / tw;
        single.texBox[5] =(ity + bfsy) / th;
        single.texBox[6] =itx / tw;
        single.texBox[7] =(ity + bfsy) / th;
        tcb.compiled.push_back(single);
      }
      compiled.push_back(tcb);
    }
    return compiled;
  };

  for(int i = 0; i < _layers.size(); ++i){
    nite::TileLayer &current = _layers[i];
    Vector<TileBatchCompiled> compiled = split(current);
    TileLayerBatchBuffer batch;
    for(int c = 0; c < compiled.size(); ++c){
      nite::TextureRegionBatch trb;
      trb.position.set(0.0);
      trb.size.set(tileSize);
      trb.depth = compiled[c].depth;
      for(int j = 0; j < compiled[c].compiled.size(); ++j){
        trb.regions.push_back(compiled[c].compiled[j]);
      }
      batch.batches.push_back(trb);
    }
    tileBaches.push_back(batch);
  }
  nite::print("loaded tileset data from '"+path+"'. Time: "+nite::toStr(nite::getTicks()-startTime)+" msecs.");
}

nite::Renderable *nite::Tileset::draw(float x, float y){
  for(int i = 0; i < tileBaches.size(); ++i){
    for(int c = 0; c < tileBaches[i].batches.size(); ++c){
      nite::TextureRegionBatch &ref = tileBaches[i].batches[c];
      nite::setDepth(ref.depth);
      source.draw(ref, 0.0f, 0.0f);
    }
  }
  nite::setDepth(nite::DepthMiddle);
  return NULL;
}

nite::Renderable *nite::Tileset::draw(float x, float y, float scaleX, float scaleY){

}

nite::Renderable *nite::Tileset::draw(const nite::Vec2 &position){
  return this->draw(position.x, position.y);
}

nite::Renderable *nite::Tileset::draw(const nite::Vec2 &position, const nite::Vec2 &scale){
  return this->draw(position.x, position.y, scale.x, scale.y);
}

void nite::Tileset::clear(){
  tileBaches.clear();
}
