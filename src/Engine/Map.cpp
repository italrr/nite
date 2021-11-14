#include <algorithm>

#include "Tools/Tools.hpp"
#include "View.hpp"
#include "Map.hpp"
#include "Texture.hpp"
#include "Console.hpp"


static bool renMap = true;
static nite::Console::CreateProxy cpAnDatTo("ren_enable_map", nite::Console::ProxyType::Bool, sizeof(bool), &renMap);

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


void nite::MapLayer::clear(){
    if(cells == NULL){
        return;
    }
    delete cells;
    this->total = 0;
    this->w = 0;
    this->h = 0;
    batch.clear();
}

nite::MapLayer::MapLayer(){
    cells = NULL;
    dynamicY = false;
    visible = true;
}

void nite::MapLayer::generate(int w, int h, int dvalue, int depth, const String &srcHash, const nite::Texture &src){
    total = w * h;
    cells = new Int32[total];
    this->depth = depth;
    this->w = w;
    this->h = h;
    this->srcHash = srcHash;
    this->src = src;
    this->type = "general";
    for(int i = 0; i < total; ++i){
        cells[i] = dvalue;
    }
}


nite::Map::Map(){
    title = "";
    author = "";
    hash = "";
    lastDTId = 0;
}

nite::Map::~Map(){
    // unload();    
}

bool buildTiledMap(nite::Map *target, const Jzon::Node &node, const String &path){
    // nite::Vec2 mapSize, tileSize, imageSize, tileMapSize;
    // float spacing, margin;
    // String sourceFilename;

    target->size.set(node.get("width").toInt(), node.get("height").toInt());
    target->tileSize.set(node.get("tilewidth").toInt(), node.get("tileheight").toInt());    

    // Read first tileset
    Jzon::Node tilesets = node.get("tilesets").get(0);
    target->spacing = tilesets.get("spacing").toFloat();
    target->margin = tilesets.get("margin").toFloat();
    // target->tileSize = 

    nite::MapTilesetSource tilesetSource;
    // sourceFilename = nite::getFilePath(path)+"/"+tilesets.get("image").toString();
    auto imageSize = nite::Vec2(tilesets.get("imagewidth").toFloat(), tilesets.get("imageheight").toFloat());
    auto tileMapSize = nite::Vec2(imageSize / (target->tileSize + target->spacing));

    tilesetSource.filename =  nite::getFilePath(path)+"/"+tilesets.get("image").toString();; // relative to the client/server
    tilesetSource.hash = nite::hashFile(tilesetSource.filename); // also the id
    tilesetSource.size = nite::fileSize(tilesetSource.filename); // in bytes;
    tilesetSource.bsize = imageSize; //in pixels
    tilesetSource.units = nite::round(imageSize / (target->tileSize + target->spacing));
    tilesetSource.src.load(tilesetSource.filename); // local copy used for actually rendering

    target->sources[tilesetSource.hash] = tilesetSource;


    target->setup(0, target->size, target->tileSize, target->margin, target->spacing);

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

    // Vector <nite::MapLayer*> _layers;
    bool isThereYSorts = false;
    // Read Layers
    for(int i = 0; i < layers.getCount(); ++i){
        if(layers.get(i).get("type").toString() != "tilelayer") continue;
        auto layer = new nite::MapLayer();
        // layer->generate()
        layer->position = nite::Vec2(layers.get(i).get("x").toFloat(), layers.get(i).get("y").toFloat());
        layer->depth = layers.get(i).get("properties").get("depth").toInt();
        layer->depthTest = layers.get(i).get("properties").get("depth_test").toBool();
        layer->name = layers.get(i).get("name").toString();
        layer->visible = layers.get(i).get("visible").toBool();
        layer->ySort  = layers.get(i).get("properties").get("y_sort").toBool();
        layer->floorDepth  = layers.get(i).get("properties").get("floor_depth").toBool();
        layer->generate(target->size.x, target->size.y, -1, layer->depth, tilesetSource.hash, tilesetSource.src);
        if(layer->ySort) isThereYSorts = true;
        Jzon::Node data = layers.get(i).get("data");
        layer->batch.setSize(target->size.x, target->size.y, target->size.x * target->tileSize.x, target->size.y * target->tileSize.y);    
        for(int c = 0; c < data.getCount(); ++c){
            float p = data.get(c).toInt();
            if(p == 0) continue;
            // float tileY = nite::floor((p-1) / (tileMapSize.x));
            // float tileX = (p-1) - tileY * (tileMapSize.x);
            // float inY = nite::floor(c / target->size.x);
            // float inX = c - inY * target->size.x;
            // int depth = 0;
            // nite::Vec2 inTextureCoors;
            // nite::Vec2 inMapCoors;
            // inTextureCoors.x = tileX * (target->tileSize.x + target->spacing.x) + target->margin.x;
            // inTextureCoors.y = tileY * (target->tileSize.y + target->spacing.y) + target->margin.y;
            // inMapCoors.x = inX * target->tileSize.x;
            // inMapCoors.y = inY * target->tileSize.y;
            // if(checkBlock(inMapCoors, layer->name)){
            //     DepthMask currentMask = getMask(inMapCoors, layer->name);
            //     depth = currentMask.correct(inMapCoors);
            // }
            nite::TextureRegionSingle single;


            int tw = imageSize.x; // Texture width
            int th = imageSize.y; // Texture height


            float inY = nite::floor(c  / target->size.x);
            float inX = c - inY * target->size.x;

            float tileY = nite::floor((p-1) / (tileMapSize.x));
            float tileX = (p-1) - tileY * (tileMapSize.x);


            float ity =  tileY* (target->tileSize.x + target->spacing.x) + target->margin.x; // inTextureCoors y
            float itx =  tileX* (target->tileSize.y + target->spacing.y) + target->margin.y; // inTextureCoors x

            // nite::print(nite::Vec2(itx, ity).str()+" "+nite::toStr(c));
            
     
            
            float bfsx = target->tileSize.x;
            float bfsy = target->tileSize.y;
            float dx = inX * target->tileSize.x; // in draw coors px
            float dy = inY * target->tileSize.y; // in draw coors py
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


            layer->batch.add(c, &single); 

            
        //   layer.data.push_back(TileLayerSingle(inTextureCoors, inMapCoors, depth, (int)p));
        //   layer->batch.add(j, &single);  
        }
        target->layers.push_back(layer);
    }


    std::sort(target->layers.begin(), target->layers.end(), [](nite::MapLayer *a, nite::MapLayer *b){
        return a->depth < b->depth;
    });

    return true;
}

bool buildNiteMap(nite::Map *target, const Jzon::Node &node, const String &path){

    struct Vec2Index : nite::Vec2 {
        int index;
    };

    auto readVec2Obj = [](const Jzon::Node &node, nite::Vec2 &holder){
        if(node.has("x") && node.get("x").isNumber()) holder.x = node.get("x").toInt();
        if(node.has("y") && node.get("y").isNumber()) holder.y = node.get("y").toInt();
    };

    auto readCellObj = [](const Jzon::Node &node, Int32 &x, Int32 &y, Int32 &index){
        if(node.has("x") && node.get("x").isNumber()) x = node.get("x").toInt();
        if(node.has("y") && node.get("y").isNumber()) y = node.get("y").toInt();
        if(node.has("i") && node.get("i").isNumber()) index = node.get("i").toInt();
    };    

    target->title = node.get("title").toString("missing");
    target->author = node.get("author").toString("missing");
    target->version = node.get("version").toString("missing");
    readVec2Obj(node.get("tileSize"), target->tileSize);
    readVec2Obj(node.get("spacing"), target->spacing);
    readVec2Obj(node.get("margin"), target->margin);
    readVec2Obj(node.get("size"), target->size);
    readCellObj(node.get("start"), target->startCell.x, target->startCell.y, target->startCell.index);
    readCellObj(node.get("end"), target->endCell.x, target->endCell.y, target->endCell.index);
    target->setup(0, target->size, target->tileSize, target->margin, target->spacing);
    // sources
    auto sourcesNode = node.get("source");
    auto indexer = nite::getIndexer();
    for(auto &src : sourcesNode){
        String hash = src.first;
        auto file = indexer->get(hash);
        String path = "";
        if(file != NULL){
            path = file->path;
        }
        nite::Vec2 bsize;
        readVec2Obj(src.second.get("size"), bsize);
        if(!nite::fileExists(path)){
            nite::print("failed to load source '"+path+"': it doesn't exist");
            continue;
        }
        if(nite::hashFile(path) != hash){
            nite::print("failed to load source '"+path+"': hashes don't match");
            continue;
        }
        nite::Texture tileset(path);
        size_t size = nite::fileSize(path);
        nite::MapTilesetSource source;
        source.bsize = bsize;
        source.filename = path;
        source.hash = hash;
        source.src = tileset;
        source.units = bsize / target->tileSize;
        target->sources[hash] = source;
    }
    

    // layers
    auto layerNode = node.get("layer");
    for(int i = 0; i < layerNode.getCount(); ++i){
        auto node = layerNode.get(i);
        String srcHash = node.get("source").toString("missing");
        bool dynamicY = node.get("dynamicY").toBool(false);
        int depth = node.get("depth").toInt(i);
        String type = node.get("type").toString("general");
        if(target->sources.find(srcHash) == target->sources.end()){
            nite::print(srcHash);
            nite::print("failed to load layer '"+nite::toStr(i)+"' from '"+path+"': didn't find matching source");
            target->unload();
            return false;
        }
        auto &srcref = target->sources[srcHash];
        auto layer = new nite::MapLayer();
        layer->generate(target->size.x, target->size.y, -1, depth, srcHash, srcref.src);
        layer->type = type;
        layer->depth = depth;
        layer->dynamicY = dynamicY;
        layer->batch.setSize(target->size.x, target->size.y, target->size.x * target->tileSize.x, target->size.y * target->tileSize.y);
        auto cellsNode = node.get("cells");
        for(int j = 0; j < cellsNode.getCount(); ++j){ // must be exact same size, otherwise segfault or ub. TODO: add checks here maybe?
            auto cell = cellsNode.get(j);
            int v = cell.get("v").toInt(-1);
            // index/value
            // layer->cells[j].val = v;
            // // pre-computed stuff
            // // in texture coors
            // if(v != -1){
            //     layer->cells[j].vy = (v - 1) / (int)srcref.units.x;
            //     layer->cells[j].vx = (v - 1) - layer->cells[j].vy * (int)srcref.units.x;
            //     layer->cells[j].vy *= tileSize.y;
            //     layer->cells[j].vx *= tileSize.x;                
            // }
            // // in map coors
            // layer->cells[j].imx = (j % (int)size.x) * tileSize.x;
            // layer->cells[j].imy = (j / (int)size.x) * tileSize.y;  

            if(v == -1){
                continue;
            }
            layer->cells[j] = v;
            nite::TextureRegionSingle single;
            int tw = srcref.bsize.x; // Texture width
            int th = srcref.bsize.y; // Texture height
            float ity = (v - 1) / (int)srcref.units.x; // inTextureCoors y
            float itx = (v - 1) - ity * (int)srcref.units.x; // inTextureCoors x
            ity *= target->tileSize.y;
            itx *= target->tileSize.x;
            float bfsx = target->tileSize.x;
            float bfsy = target->tileSize.y;
            float dx = (j % (int)target->size.x) * target->tileSize.x; // in draw coors px
            float dy = (j / (int)target->size.x) * target->tileSize.y; // in draw coors py
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
            layer->batch.add(j, &single);    
                    
        }
        target->layers.push_back(layer);
    }
    std::sort(target->layers.begin(), target->layers.end(), [](nite::MapLayer *a, nite::MapLayer *b){
        return a->depth < b->depth;
    });

    // masks
    auto masksNode = node.get("masks");
    for(int i = 0; i < masksNode.getCount(); ++i){
        auto node = masksNode.get(i);
        nite::MapWallMask mask;
        mask.position.x = node.get("x").toFloat(-1);
        mask.position.y = node.get("y").toFloat(-1);
        mask.size.x = node.get("w").toFloat(-1);
        mask.size.y = node.get("h").toFloat(-1);        
        mask.index = node.get("i").toInt(-1);        
        target->masks.push_back(mask);
    }

    // dynamic tiles
    auto dyTilesNode = node.get("dynamicTiles");
    for(auto ins : dyTilesNode){
        auto dytile = nite::MapDynamicTileGroup();
        dytile.id = nite::toInt(ins.first);
        dytile.layer = ins.second.get("layer").toInt(0);
        dytile.state = ins.second.get("state").toInt(0);        
        for(auto tls : ins.second.get("tiles")){
            dytile.tiles.push_back(tls.second.toInt(0));
        }
        for(auto sts : ins.second.get("stateToIndex")){
            dytile.stateToIndex[nite::toInt(sts.first)] = sts.second.toInt(0);
        }
        target->dynamicTiles[dytile.id] = dytile;
        target->setDynamicTileState(dytile.id, 0);
    }

    // special descriptors
    auto specialsNode = node.get("specialObjects");
    for(auto ins : specialsNode){
        auto sp = nite::MapSpecialDescriptor();
        sp.id = nite::toInt(ins.first);
        sp.tag = ins.second.get("tag").toString("");
        sp.ref = ins.second.get("ref").toInt(0);        
        sp.type = ins.second.get("type").toString("");
        sp.position.x = ins.second.get("x").toFloat(0);
        sp.position.y = ins.second.get("y").toFloat(0);
        sp.size.x = ins.second.get("w").toFloat(0);
        sp.size.y = ins.second.get("h").toFloat(0);
        target->specials[sp.id] = sp;
    }

    // TODO: `nav`
    // TODO: `fragments`
    return true;
}

bool nite::Map::load(const String &path){
    if(!nite::fileExists(path)){
        nite::print("failed to load nite::Map: '"+path+"' doesn't exist");
        return false;
    }
    Jzon::Parser parser;
    auto file = parser.parseFile(path);
    if(!file.isValid()){
        nite::print("failed to parse nite::Map '"+path+"': "+parser.getError());
        return false;
    }
    unload();

    // detect map type
    auto mapFileType = file.get("__NITE_MAP_VERSION__").toString("0"); // nite maps must include version 1 at the very least
    
    // Tiled Map
    if(mapFileType == "0"){
        nite::print("loading Tiled map type...");
        return buildTiledMap(this, file, path);
    // nite Map
    }else{
        return buildNiteMap(this, file, path);
    }
}


bool nite::Map::compose(const nite::Map &fragment){
    // TODO
    return false;
}

bool nite::Map::setup(int n, const nite::Vec2 &size, const nite::Vec2 &tileSize, const nite::Vec2 &margin, const nite::Vec2 &spacing){
    this->size = size;
    this->tileSize = tileSize;
    this->margin = margin;
    this->spacing = spacing;
    this->total = size.x * size.y;
    if(tileSize.x == 0 || tileSize.y == 0){
        nite::print("failed to setup nite::Map: tileSize cannot be 0");
        return false;
    }
    if(size.x == 0 || size.y == 0){
        nite::print("failed to setup nite::Map: size cannot be 0");
        return false;
    }    
    for(int i = 0; i < n; ++i){
        auto layer = new MapLayer();
        layer->generate(size.x, size.y, -1, i, "", nite::Texture());
        layers.push_back(layer);
    }
    empty.load("data/texture/empty.png"); 
    return true;
}

bool nite::Map::add(int *cells, int depth, const String &src, const String &type, bool dynamicY){
    if(size.x == 0 || size.y == 0){
        nite::print("failed to load layer on nite::Map: map must be initialized with setup() first");
        return false;
    }        
    if(!nite::fileExists(src)){
        nite::print("failed to load layer on nite::Map: src '"+src+"' does not exist");
        return false;
    } 
    if(depth < 0 || depth > layers.size()){
        nite::print("failed to load layer on nite::Map: depth '"+nite::toStr(depth)+"' goes off bounds");
        return false;
    }    
    String hash = nite::hashFile(src);
    nite::MapTilesetSource *tilesrc = NULL;
    nite::Texture tileset;
    nite::MapLayer *layer = NULL;
    // if source doesn't exist, add it
    if(sources.find(hash) == sources.end()){
        tileset.load(src);
        MapTilesetSource tmp;
        tmp.hash = hash;
        tmp.filename = src;
        tmp.src = tileset;
        tmp.size = nite::fileSize(src);
        tmp.bsize = tileset.getSize();
        tmp.units = tileset.getSize() / tileSize;
        sources[hash] = tmp;       
    }
    tilesrc = &sources[hash];
    tileset = tilesrc->src;
    bool foundDepth = false;
    // look for a layer with this depth
    for(int i = 0; i < layers.size(); ++i){
        if(layers[i]->depth == depth){
            layer = layers[i];
            foundDepth = true;
            break;
        }
    }
    // if none was found, add it and sort it
    if(!foundDepth){
        layer = new nite::MapLayer();
        layer->generate(size.x, size.y, -1, depth, hash, tileset);
        layers.push_back(layer);
        // sort everything afterwards
        std::sort(layers.begin(), layers.end(), [](nite::MapLayer *a, nite::MapLayer *b){
            return a->depth < b->depth;
        });
    }
    layer->type = type;
    layer->dynamicY = dynamicY;
    layer->src = tilesrc->src;
    layer->srcHash = hash;
    layer->batch.setSize(size.x, size.y, size.x * tileSize.x, size.y * tileSize.y);
    // copy layer
    for(int i = 0; i < total; ++i){
        // index/value
        // layer->cells[i].val = cells[i];
        // // in texture coors
        // layer->cells[i].vy = (cells[i] - 1) / (int)tilesrc->units.x;
        // layer->cells[i].vx = (cells[i] - 1) - layer->cells[i].vy * (int)tilesrc->units.x;
        // layer->cells[i].vy *= tileSize.y;
        // layer->cells[i].vx *= tileSize.x;
        // // in map coors
        // layer->cells[i].imx = (i % (int)size.x) * tileSize.x;
        // layer->cells[i].imy = (i / (int)size.x) * tileSize.y;   
        if(cells[i] == -1){
            continue;
        }
        layer->cells[i] = cells[i];
        nite::TextureRegionSingle single;
        int tw = tilesrc->bsize.x; // Texture width
        int th = tilesrc->bsize.y; // Texture height
        float ity = (cells[i] - 1) / (int)tilesrc->units.x; // inTextureCoors y
        float itx = (cells[i] - 1) - ity * (int)tilesrc->units.x; // inTextureCoors x
        ity *= tileSize.y;
        itx *= tileSize.x;
        float bfsx = tileSize.x;
        float bfsy = tileSize.y;
        float dx = (i % (int)size.x) * tileSize.x; // in draw coors px
        float dy = (i / (int)size.x) * tileSize.y; // in draw coors py
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
        layer->batch.add(i, &single);
                     
    }
    return true;
}

void nite::Map::unload(){
    for(int i = 0; i < layers.size(); ++i){
        layers[i]->clear();
    }
    for(int i = 0; i < layers.size(); ++i){
        delete layers[i];
    }
    sources.clear();
    empty.unload();
}

bool nite::Map::exportToJson(const String &path, bool allowOverwrite){
    UInt64 initTime = nite::getTicks();
    if(nite::fileExists(path)){
        if(allowOverwrite){
            nite::print("map file '"+path+"' already exists: will be overwritten");
        }else{
            nite::print("map file '"+path+"' already exists: cannot overwrite");
            return false;
        }
    }

    auto buildVec2Obj = [](float x, float y, int index = -1){
        Jzon::Node node = Jzon::object();
        node.add("x", x);
        node.add("y", y);
        if(index != -1){
            node.add("i", index);
        }
        return node;
    };

    Jzon::Writer writer;
    Jzon::Node file = Jzon::object();

    // meta
    file.add("title", title);
    file.add("author", author);
    file.add("version", version);
    file.add("tileSize", buildVec2Obj(tileSize.x, tileSize.y));
    file.add("spacing", buildVec2Obj(spacing.x, spacing.y));
    file.add("margin", buildVec2Obj(margin.x, margin.y));
    file.add("size", buildVec2Obj(size.x, size.y));
    file.add("start", buildVec2Obj(startCell.x, startCell.y, startCell.index));
    file.add("end", buildVec2Obj(endCell.x, endCell.y, startCell.index));
    file.add("__NITE_MAP_VERSION__", "1");
    // sources
    Jzon::Node sourcesObj = Jzon::object();
    for(auto &src : sources){
        Jzon::Node srcObj = Jzon::object();
        srcObj.add("size", buildVec2Obj(src.second.bsize.x, src.second.bsize.y)); // size of the bitmap itself
        sourcesObj.add(src.first, srcObj);
    }
    file.add("source", sourcesObj);
    String defaultSrc = sources.begin()->first;
    // layers
    Jzon::Node layersArr = Jzon::array();
    for(int i = 0; i < layers.size(); ++i){
        auto layer = layers[i];
        Jzon::Node layerObj = Jzon::object();
        Jzon::Node cellsArr = Jzon::array();
        layerObj.add("source", layer->srcHash);
        layerObj.add("depth", layer->depth);
        layerObj.add("type", layer->type);
        if(layer->dynamicY){
            layerObj.add("dynamicY", true);
        }
        for(int j = 0; j < layer->total; ++j){
            Jzon::Node cellObj = Jzon::object();
            cellObj.add("v", layer->cells[j]);
            cellsArr.add(cellObj);
        }
        layerObj.add("cells", cellsArr);
        layersArr.add(layerObj);
    }
    file.add("layer",layersArr);
    // masks
    Jzon::Node masksArr = Jzon::array();
    for(int i = 0; i < masks.size(); ++i){
        Jzon::Node maskObj = Jzon::object();
        maskObj.add("x", masks[i].position.x);
        maskObj.add("y", masks[i].position.y);
        maskObj.add("w", masks[i].size.x);
        maskObj.add("h", masks[i].size.y);
        maskObj.add("i", masks[i].index);
        masksArr.add(maskObj);
    }
    file.add("masks", masksArr);
    // dynamic tiles
    Jzon::Node dyTiles = Jzon::object();
    for(auto &it : dynamicTiles){
        auto &dty = it.second;
        auto obj = Jzon::object();
        auto tiles = Jzon::array();
        auto stateToIndex = Jzon::object();
        for(auto st : dty.stateToIndex){
            stateToIndex.add(nite::toStr(st.first), st.second);
        }
        for(int i = 0; i < dty.tiles.size(); ++i){
            tiles.add(dty.tiles[i]);
        }
        obj.add("state", dty.state);
        obj.add("layer", dty.layer);
        obj.add("tiles", tiles);
        obj.add("stateToIndex", stateToIndex);
        dyTiles.add(nite::toStr(it.first), obj);
    }
    file.add("dynamicTiles", dyTiles);
    // specials
    Jzon::Node spGroups = Jzon::object();
    for(auto &it : specials){
        auto &sp = it.second;
        auto obj = Jzon::object();
        obj.add("id", it.first);
        obj.add("ref", it.second.ref);
        obj.add("tag", it.second.tag);
        obj.add("type", it.second.type);
        obj.add("x", it.second.position.x);
        obj.add("y", it.second.position.y);
        obj.add("w", it.second.size.x);
        obj.add("h", it.second.size.y);
        spGroups.add(nite::toStr(it.first), obj);
    }    
    file.add("specialObjects", spGroups);

    //TODO: `nav`
    //TODO: `fragments`

    writer.writeFile(file, path);
    nite::print("saved map '"+title+"' to '"+path+"' | time: "+nite::toStr(nite::getTicks()-initTime)+" msecs");
    return true;
}

int nite::Map::generateDynamicTile(const Dict<int, int> &stateToIndex, const Vector<int> &tiles, int layer, int setId){
    int cid =  setId == -1 ? lastDTId++ : setId;
    MapDynamicTileGroup gp;
    gp.stateToIndex = stateToIndex;
    gp.tiles = tiles;
    gp.id = cid;
    gp.state = 0;
    gp.layer = layer;
    dynamicTiles[cid] = gp;
    return cid;
}

int nite::Map::generateDescriptor(const nite::Vec2 &p, const nite::Vec2 &s, const String &type, const String &tag, int ref, int setId){
    int cid =  setId == -1 ? lastDTId++ : setId;
    MapSpecialDescriptor sp;
    sp.ref = ref;
    sp.id = cid;
    sp.position = p;
    sp.size = s;
    sp.tag = tag;
    sp.type = type;
    specials[cid] = sp;
    return cid;
}

void nite::Map::resetDynamicTile(int id){
    auto it = dynamicTiles.find(id);
    if(it == dynamicTiles.end()){
        return;
    }
    setDynamicTileState(it->second.id, 0);
}

void nite::Map::setDynamicTileState(int id, int state){
    auto &dytiles = dynamicTiles[id];
    int nindx = dytiles.stateToIndex[state];
    for(int j = 0; j < layers.size(); ++j){
        auto &layer = layers[j];
        if(layer->depth != dytiles.layer){
            continue;
        }
        for(int i = 0; i < dytiles.tiles.size(); ++i){
            layer->cells[dytiles.tiles[i]] = nindx;
            nite::TextureRegionSingle *single = (nite::TextureRegionSingle*)(layer->batch.cells + dytiles.tiles[i] * sizeof(nite::TextureRegionSingle));
            auto *tilesrc = &sources[layer->srcHash];
            int tw = tilesrc->bsize.x; // Texture width
            int th = tilesrc->bsize.y; // Texture height
            float ity = (nindx - 1) / (int)tilesrc->units.x; // inTextureCoors y
            float itx = (nindx - 1) - ity * (int)tilesrc->units.x; // inTextureCoors x
            ity *= tileSize.y;
            itx *= tileSize.x;
            float bfsx = tileSize.x;
            float bfsy = tileSize.y;
            float dx = (dytiles.tiles[i] % (int)size.x) * tileSize.x; // in draw coors px
            float dy = (dytiles.tiles[i] / (int)size.x) * tileSize.y; // in draw coors py
            single->box[0] = dx;
            single->box[1] = dy;
            single->box[2] = dx + bfsx;
            single->box[3] = dy;
            single->box[4] = dx + bfsx;
            single->box[5] = dy + bfsy;
            single->box[6] = dx;
            single->box[7] = dy + bfsy;
            single->texBox[0] = itx / tw;
            single->texBox[1] = ity / th;
            single->texBox[2] = (itx + bfsx) / tw;
            single->texBox[3] = ity / th;
            single->texBox[4] = (itx + bfsx) / tw;
            single->texBox[5] = (ity + bfsy) / th;
            single->texBox[6] = itx / tw;
            single->texBox[7] = (ity + bfsy) / th;            
        }
    }
}

void nite::Map::render(const nite::Vec2 &pos, const nite::Rect &vwp){
    if(layers.size() == 0){
        return;
    }
    nite::setDepth(nite::DepthMiddle);
    nite::setRenderTarget(nite::RenderTargetPrevious);   // render black background right before Game
    nite::setColor(0.0f, 0.0f, 0.0f, 1.0f); 
    empty.draw(0.0f, 0.0f, nite::getWidth(), nite::getHeight(), 0.0f, 0.0f, 0.0f);
    if(!renMap){
        return;
    }
    nite::setDepth(nite::DepthMiddle);
    nite::setRenderTarget(nite::RenderTargetGame);   
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f); 
    nite::Vec2 realSize = tileSize * size;
    int vxdiff = nite::round((vwp.x / realSize.x) * size.x);
    int vydiff = nite::round((vwp.y / realSize.y) * size.y);       
    int vwdiff = nite::round((vwp.w / realSize.x) * size.x);
    int vhdiff = nite::round((vwp.h / realSize.y) * size.y);           
    for(int j = 0; j < layers.size(); ++j){
        
        if(!layers[j]->visible){
            continue;
        }
        nite::setDepth(layers[j]->dynamicY ? nite::DepthTop : nite::DepthMiddle);
        layers[j]->src.drawCellBatch(&layers[j]->batch, layers[j]->position.x +  pos.x, layers[j]->position.y + pos.y, vxdiff, vydiff, vwdiff, vhdiff);    
    }
    nite::setDepth(nite::DepthMiddle);
}

