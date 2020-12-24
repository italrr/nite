#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "../Engine/Shapes.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"

#include "RING.hpp"

static Game::RING::Criteria criteria;

struct MappingCriteria {
    Vector<int> region;
    String key;    
    void pull(int index, int *grid, int width, int height){ // copy cells 
        for(int i = 0; i < criteria.sampleSize; ++i){
            int xrel = i % criteria.width;
            int yrel = i / criteria.width;
            int gridx = index % width;
            int gridy = index / width;
            int xg = gridx - criteria.offsetx;
            int yg = gridy - criteria.offsety;
            int xi = (xg + xrel);
            int yi = (yg + yrel);
            if(xi < 0 || yi < 0 || xi >= width || yi >= height){  // out of bouds
                this->region[i] = criteria.outbounds;
                continue;
            }            
            this->region[i] = grid[xi + yi * width];
        }
    }
    void loadFromRule(const Game::RING::Rule &rule){
        this->key = rule.k;
        this->region = rule.v;
    }    
    bool match(const MappingCriteria &other){
        for(int i = 0; i < criteria.sampleSize; ++i){
            if(this->region[i] == criteria.any){
                continue;
            }            
            if(this->region[i] != other.region[i]){
                return false;
            }
        }
        return true;
    }    
    void set(const Vector<int> &region, const String &key){
        this->key = key;
        for(int i = 0; i < criteria.sampleSize; ++i){
            this->region[i] = region[i];
        }
    }
    MappingCriteria(const Vector<int> &region, const String &key){
        this->region = Vector<int>(criteria.sampleSize, -1);
        set(region, key);
    }
    MappingCriteria(int index, int *grid, int width, int height){
        this->region = Vector<int>(criteria.sampleSize, -1);
        pull(index, grid, width, height);
    }    
    MappingCriteria(){
        this->region = Vector<int>(criteria.sampleSize, -1);
    }
    operator std::string() const{
        String output;
        for(int i = 0; i < criteria.sampleSize; ++i){
            String lett = "";
            if(this->region[i] == Game::RING::CellType::Path){
                lett = "P";
            }else
            if(this->region[i] == Game::RING::CellType::Wall){
                lett = "W";
            }else
            if(this->region[i] == criteria.any){
                lett = "A";
            }else
            if(this->region[i] == criteria.outbounds){
                lett = "O";
            }
            output += lett+" ";
            if(i % criteria.width == criteria.width-1){
                output += "\n";
            }
        }
        if(this->key.length() > 0){
            output += "\n'"+this->key+"'";
        }
        return  output;
    }
};

static Vector<MappingCriteria> rules;

static MappingCriteria matchForRules(const MappingCriteria &src){
    for(int i = 0; i < rules.size(); ++i){
        auto &rule = rules[i];
        if(rule.match(src)){
            return rule;
        }
    }
    MappingCriteria __default;
    __default.key = "";
    for(int i = 0; i < criteria.sampleSize; ++i){
        __default.region.push_back(-1);
    }
    return __default;
}
static void loadFromRules(const Vector<Game::RING::Rule> &_rules){
    rules.clear();
    for(int i = 0; i < _rules.size(); ++i){
        MappingCriteria mpr;
        mpr.loadFromRule(_rules[i]);
        rules.push_back(mpr);
    }
    nite::print("loaded "+nite::toStr(rules.size())+" MappingCriteria rule(s)");
}

Game::RING::TileSource::TileSource(const String &path){
    load(path);
}

Game::RING::TileSource::TileSource(){

}

void Game::RING::TileSource::load(const String &path){
    Jzon::Parser parser;
    Jzon::Node node = parser.parseFile(path);
    if (!node.isValid()){
        nite::print("failed to load json "+path+": '"+parser.getError()+"'");
        return;
    }
    this->name = node.get("name").toString();
    this->description = node.get("description").toString();
    this->sourcePath = node.get("source").toString();
    this->transparency.set(nite::Color(node.get("transparency")));
    this->source.load(this->sourcePath, this->transparency);
    this->sourceSize.set(node.get("sourceWidth").toFloat(), node.get("sourceHeight").toFloat());
    this->margin = node.get("margin").toFloat();
    this->spacing = node.get("spacing").toFloat();
    this->tileSize.set(node.get("tileWidth").toFloat(), node.get("tileHeight").toFloat());
    this->floorDefault = node.get("floorDefault").toString();
    this->floorVarianceFactor = node.get("floorVarianceFactor").toFloat();
    this->criteria.load(node.get("criteria"));
    this->lastFloorVariant = floorDefault;
    auto mappings = node.get("mapping");
    for(auto mapp : mappings){
        this->mapping[mapp.first] = mapp.second.toInt();
    }
    // TODO: add checks key against mappings    
    if(node.has("ignoresForFloor") && node.get("ignoresForFloor").isArray()){
        for(int i = 0; i < node.get("ignoresForFloor").getCount(); ++i){
            this->ignoresForFloor.push_back(node.get("ignoresForFloor").get(i).toString());
        }
    }
    if(node.has("dynamicYDepth") && node.get("dynamicYDepth").isArray()){
        for(int i = 0; i < node.get("dynamicYDepth").getCount(); ++i){
            this->dynamicYDepth.push_back(node.get("dynamicYDepth").get(i).toString());
        }
    }    
    if(node.has("floorVariants") && node.get("floorVariants").isArray()){
        for(int i = 0; i < node.get("floorVariants").getCount(); ++i){
            this->floorVariants.push_back(node.get("floorVariants").get(i).toString());
        }
    }
    if(node.has("specialWallMasks") && node.get("specialWallMasks").isObject()){
        auto objs = node.get("specialWallMasks");
        for(auto obj : objs){
            Game::RING::TileSourceWalkMask mask;
            mask.key = obj.first;
            mask.size.x = obj.second.get("width").toFloat();
            mask.size.y = obj.second.get("height").toFloat();
            mask.offset.x = obj.second.get("xOffset").toFloat();
            mask.offset.y = obj.second.get("yOffset").toFloat();            
            this->specialWallMasks.push_back(mask);
        }        
    }
    if(node.has("arbitraryVariance") && node.get("arbitraryVariance").isArray()){
        auto vars = node.get("arbitraryVariance");
        for(auto obj : vars){
            Game::RING::ArbirtraryVariance vari;
            vari.target = obj.second.get("target").toString("");
            vari.variant = obj.second.get("variant").toString("");
            vari.chance = obj.second.get("chance").toFloat(0.0f);
            this->arbVar.push_back(vari);
        }
    }
    nite::print("loaded builder template json '"+path+"'");
}


Game::RING::TileSourceWalkMask Game::RING::TileSource::getSpecialWallMask(int mappVal){
    Game::RING::TileSourceWalkMask empty;
    for(auto mapp : this->mapping){
        if(mapp.second == mappVal){
            for(int i = 0; i < this->specialWallMasks.size(); ++i){
                if(this->specialWallMasks[i].key == mapp.first){
                    return this->specialWallMasks[i];
                }
            }
        }
    }
    return empty;
}

String Game::RING::TileSource::getFloorVariant(Game::RING::Blueprint &bp){
    bool jackpot = nite::randomInt(1, 100) < 100 * this->floorVarianceFactor;
    if(floorVariants.size() == 0 || !jackpot){
        return floorDefault;
    }
    String current = floorDefault;
    while(current.length() == 0 || current == lastFloorVariant){
        current = floorVariants[nite::randomInt(0, floorVariants.size() - 1)];
    }
    lastFloorVariant = current;
    return current;
}

bool Game::RING::TileSource::isDynamicYDepth(const String &key){
    for(int i = 0; i < dynamicYDepth.size(); ++i){
        if(dynamicYDepth[i] == key){
            return true;
        }
    }
    return false;
}

bool Game::RING::TileSource::isIgnoredForFLoors(const String &key){
    for(int i = 0; i < ignoresForFloor.size(); ++i){
        if(ignoresForFloor[i] == key){
            return true;
        }
    }
    return false;
}

// static MappingCriteria *stored = NULL;
// static int storedtotal = 0;
// static int storedwith = 0;
// static nite::Vec2 storedtilesize;
// void __temp(){
//     for(int i = 0; i < storedtotal; ++i){
//         nite::Vec2 mrp = nite::mousePosition() + nite::getView(nite::RenderTargetGame);
//         int inx = (i %  storedwith) * storedtilesize.x;
//         int iny = (i / storedwith) * storedtilesize.y;
        
//         bool inRegion = nite::isPointInRegion(mrp, nite::Vec2(inx, iny), nite::Vec2(inx, iny) + nite::Vec2(128.0f));

//         if(inRegion && nite::mousePressed(nite::butLEFT)){
//             std::cout << (String)stored[i] << std::endl;
//         }
//     }
// }


Shared<nite::Map> Game::RING::generateMap(Shared<Game::RING::Blueprint> bp, Game::RING::TileSource &temp, int scale){
    UInt64 initTime = nite::getTicks();
    auto width = bp->width * scale;
    auto height = bp->height * scale;
    auto size = width * height;
    auto map = Shared<nite::Map>(new nite::Map());
    nite::print("generating map | size "+nite::Vec2(width, height).str()+" | total "+nite::toStr(size)+" cells");

    // create layers
    int floor[size];
    int walls[size];
    int dynamicY[size];
    // int specials[size];
    int mirror[size]; // temp
    int allWalls[size]; //temp
    
    // stored = new MappingCriteria[size];
    // storedtotal = size;
    // storedwith = width;
    // storedtilesize = temp.tileSize;

    for(int i = 0; i < size; ++i){
        floor[i] = -1;
        walls[i] = -1;
        dynamicY[i] = -1;
        // specials[i] = -1;
        mirror[i] = -1;
        allWalls[i] = -1;        
    }

    // fill grid using blueprint 
    for(int i = 0; i < size; ++i){
        int pind = (i % width) / scale + ((i / width) / scale) * bp->width; // index using projected x,y
        mirror[i] = bp->grid[pind].type;
    }

    criteria = temp.criteria;
    loadFromRules(criteria.rules);

    auto findArbiVar = [&](const String &target){
        Vector<ArbirtraryVariance> matches;
        for(int i = 0; i < temp.arbVar.size(); ++i){
            if(temp.arbVar[i].target == target){
                matches.push_back(temp.arbVar[i]);
            }
        }
        if(matches.size() > 0){
            return std::pair<bool, ArbirtraryVariance>(true, matches[nite::randomInt(1, matches.size())-1]);
        }
        return std::pair<bool, ArbirtraryVariance>(false, ArbirtraryVariance());
    };

    // put walls using template
    for(int i = 0; i < size; ++i){
        auto crit = MappingCriteria(i, mirror, width, height);
        auto isMatch = matchForRules(crit);
        // stored[i] = crit;
        if(isMatch.key != ""){

            auto var = findArbiVar(isMatch.key);
            auto v = temp.mapping[isMatch.key];
            
            if(var.first && nite::randomInt(1, 100) < 100 * var.second.chance){
                v =  temp.mapping[var.second.variant];
            }

            if(temp.isDynamicYDepth(isMatch.key)){
                dynamicY[i] = v;
            }else{
                walls[i] = v;
            }
            allWalls[i] = v;
        }
        // anything that is a "path" or an inner wall gets to have a floor using ignoresForFloor for exceptions
        if(mirror[i] == Game::RING::CellType::Path || (isMatch.key.length() > 0 && !temp.isIgnoredForFLoors(isMatch.key))){ 
            floor[i] = temp.mapping[temp.getFloorVariant(*bp)];
        }        
    }
    
    // generate map
    String createdDate = nite::getTime(nite::TM::ALL);
    for(int i = 0; i < createdDate.length(); ++i){
        if(createdDate[i] == ' '){
            createdDate[i] = '-';
        }
        if(createdDate[i] == ':'){
            createdDate[i] = '_';
        }
    }
    String uuid = nite::hashString(nite::toStr(nite::getTicks()));
    map->title = "RING_"+createdDate+"_"+uuid;
    map->hash = uuid;
    map->author = "RING:none@none.com";
    map->version = "0.0.0";
    int indxs = (bp->start % bp->width) * scale;
    int indys = (bp->start / bp->width) * scale;
    map->startCell.index = indxs + indys * width;
    map->startCell.x = indxs;
    map->startCell.y = indys;
    map->setup(3, nite::Vec2(width, height), temp.tileSize, temp.margin, temp.spacing);
    map->add(floor, 0, temp.sourcePath, "floor");
    map->add(walls, 1, temp.sourcePath, "walls");
    map->add(dynamicY, 2, temp.sourcePath, "dynamicY", true); // dynamicY is usually walls
    
    // now, create masks...

    for(int i = 0; i < size; ++i){
        if(mirror[i] != Game::RING::CellType::Wall){
            continue;
        }
        auto mask = nite::MapWallMask();
        mask.size.set(temp.tileSize);
        mask.index = i;
        mask.position.x = (i % width) * temp.tileSize.x;
        mask.position.y = (i / width) * temp.tileSize.y;
        Game::RING::TileSourceWalkMask specMask = temp.getSpecialWallMask(allWalls[i]);
        if(specMask.key != ""){
            // mask.size.set(specMask.size);
            // mask.position.x += specMask.offset.x;        
            // mask.position.y += specMask.offset.y;
            continue;
        }
        // chunks[i % width][i / width] = mask;
        map->masks.push_back(mask);
        // chunksTemp.push_back(mask);
    }

    // generate traps


    // generate navegation paths
    auto generateNav = [&](){

    };


    // test trap
    // map->generateDynamicTile({ {0, 36}, {1, 37} }, {map->startCell.index + 2}, 0, 10);
    // map->generateDescriptor(nite::Vec2(map->startCell.x + temp.tileSize.x * 2, map->startCell.y), temp.tileSize, "trap", "needles", 10, 25);

    // auto table = _ptable(width, height, chunksTemp);

    // auto _xy2ind = [](int index, int w){
    //     return (index % w) + (index / w);
    // };

    // // optimize masks by joining them together
    // auto _lrj = [](int index, int w, char* chunks){
    //     int sindex = index;
    //     do {
    //         nite::MapWallMask* main = (nite::MapWallMask*)(chunks + index * sizeof(nite::MapWallMask));
    //         if(main == NULL){
    //             break;
    //         }
    //         ++sindex;
    //     } while (sindex % w != 0);
    // };

    // _lrj(0, width, table);

    // delete table;
    
    nite::print("built map: size "+nite::toStr(width)+"x"+nite::toStr(height)+"("+nite::toStr(size)+") | masks: "+nite::toStr(map->masks.size())+" | time: "+nite::toStr(nite::getTicks() - initTime)+" msecs");
    return map;
}