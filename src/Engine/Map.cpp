#include <algorithm>

#include "Tools/Tools.hpp"
#include "View.hpp"
#include "Map.hpp"
#include "Texture.hpp"
#include "Console.hpp"


static bool renMap = true;
static nite::Console::CreateProxy cpAnDatTo("ren_enable_map", nite::Console::ProxyType::Bool, sizeof(bool), &renMap);

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
}

nite::Map::~Map(){
    // unload();    
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

    title = file.get("title").toString("missing");
    author = file.get("author").toString("missing");
    version = file.get("version").toString("missing");
    readVec2Obj(file.get("tileSize"), tileSize);
    readVec2Obj(file.get("spacing"), spacing);
    readVec2Obj(file.get("margin"), margin);
    readVec2Obj(file.get("size"), size);
    readCellObj(file.get("start"), startCell.x, startCell.y, startCell.index);
    readCellObj(file.get("end"), endCell.x, endCell.y, endCell.index);
    setup(0, size, tileSize, margin, spacing);
    // sources
    auto sourcesNode = file.get("source");
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
        MapTilesetSource source;
        source.bsize = bsize;
        source.filename = path;
        source.hash = hash;
        source.src = tileset;
        source.units = bsize / tileSize;
        this->sources[hash] = source;
    }

    // layers
    auto layerNode = file.get("layer");
    for(int i = 0; i < layerNode.getCount(); ++i){
        auto node = layerNode.get(i);
        String srcHash = node.get("source").toString("missing");
        bool dynamicY = node.get("dynamicY").toBool(false);
        int depth = node.get("depth").toInt(i);
        String type = node.get("type").toString("general");
        if(sources.find(srcHash) == sources.end()){
            nite::print("failed to load layer '"+nite::toStr(i)+"' from '"+path+"': didn't find matching source");
            unload();
            return false;
        }
        auto &srcref = sources[srcHash];
        auto layer = new nite::MapLayer();
        layer->generate(size.x, size.y, -1, depth, srcHash, srcref.src);
        layer->type = type;
        layer->depth = depth;
        layer->dynamicY = dynamicY;
        layer->batch.setSize(size.x, size.y, size.x * tileSize.x, size.y * tileSize.y);
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
            ity *= tileSize.y;
            itx *= tileSize.x;
            float bfsx = tileSize.x;
            float bfsy = tileSize.y;
            float dx = (j % (int)size.x) * tileSize.x; // in draw coors px
            float dy = (j / (int)size.x) * tileSize.y; // in draw coors py
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
        this->layers.push_back(layer);
    }
    std::sort(layers.begin(), layers.end(), [](nite::MapLayer *a, nite::MapLayer *b){
        return a->depth < b->depth;
    });

    // masks
    auto masksNode = file.get("masks");
    for(int i = 0; i < masksNode.getCount(); ++i){
        auto node = masksNode.get(i);
        nite::MapWallMask mask;
        mask.position.x = node.get("x").toFloat(-1);
        mask.position.y = node.get("y").toFloat(-1);
        mask.size.x = node.get("w").toFloat(-1);
        mask.size.y = node.get("h").toFloat(-1);        
        mask.index = node.get("i").toInt(-1);        
        this->masks.push_back(mask);
    }

    // TODO: `nav`
    // TODO: `fragments`
    return true;
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
            nite::print("map file '"+path+"' already exists: failure");
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

    //TODO: `nav`
    //TODO: `fragments`

    writer.writeFile(file, path);
    nite::print("saved map '"+title+"' to '"+path+"' | time: "+nite::toStr(nite::getTicks()-initTime)+" msecs");
    return true;
}


void nite::Map::draw(const nite::Vec2 &pos, const nite::Rect &vwp){
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
        nite::setDepth(layers[j]->dynamicY ? nite::DepthTop : nite::DepthMiddle);
        layers[j]->src.drawCellBatch(&layers[j]->batch, pos.x, pos.y, vxdiff, vydiff, vwdiff, vhdiff);    
    }
    nite::setDepth(nite::DepthMiddle);
}

