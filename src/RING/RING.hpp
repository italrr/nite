#ifndef GAME_RING_HPP
    #define GAME_RING_HPP
    
    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Texture.hpp"
    #include "../Engine/Map.hpp"
    #include "Blueprint.hpp"

    /*
        [R]USH [I]N DU[N]GEONS [G]ENERATOR

        RING is the map/campaign generator.

        We'll be using different types of Maze generators at the
        beginning, but our ultimate goal is to combine the technologies.

        Our first experimentation will be using Depth-First Search.

    */

    namespace Game {
        namespace RING {

            struct TileSourceWalkMask {
                nite::Vec2 size;
                nite::Vec2 offset;
                String key;
                TileSourceWalkMask(){
                    this->key = "";
                }              
            };

            struct Rule {
                String k;
                Vector<int> v;
                void load(const Jzon::Node &obj){
                    this->k = obj.get("k").toString();
                    auto v = obj.get("v");
                    for(int i = 0; i < v.getCount(); ++i){
                        this->v.push_back(v.get(i).toInt());
                    }
                }                    
            };

            struct Criteria {
                int width;
                int height;
                int sampleSize; // width * height
                int outbounds;
                int any;
                int offsetx;
                int offsety;
                Vector<Rule> rules;
                void load(const Jzon::Node &obj){
                    width = obj.get("width").toInt();
                    height = obj.get("height").toInt();
                    sampleSize = obj.get("sampleSize").toInt();
                    any = obj.get("any").toInt();
                    outbounds = obj.get("outbounds").toInt();
                    offsetx = obj.get("offset_start_x").toInt();
                    offsety = obj.get("offset_start_y").toInt();
                    auto ruleObj = obj.get("rules");
                    rules.clear();
                    for(int i = 0; i < ruleObj.getCount(); ++i){
                        Rule rule;
                        rule.load(ruleObj.get(i));
                        rules.push_back(rule);
                    }                                        
                }
            };

            struct ArbirtraryVariance {
                String target;
                String variant;
                float chance;
            };

            struct TileSource {
                String name;
                String description;
                String sourcePath;
                nite::Texture source;
                nite::Vec2 sourceSize;
                nite::Vec2 margin;
                nite::Vec2 spacing;
                nite::Vec2 tileSize;
                Criteria criteria;
                nite::Color transparency;
                Vector<RING::ArbirtraryVariance> arbVar;
                TileSource(const String &path);
                TileSource();
                void load(const String &path);
                bool isIgnoredForFLoors(const String &key);
                bool isDynamicYDepth(const String &key);
                Dict<String, int> mapping;
                String floorDefault;
                Vector<String> floorVariants;
                Vector<String> dynamicYDepth;;
                Vector<String> ignoresForFloor;
                Vector<TileSourceWalkMask> specialWallMasks;
                String lastFloorVariant;
                String getFloorVariant(Game::RING::Blueprint &bp);
                Game::RING::TileSourceWalkMask getSpecialWallMask(int mappVal);
                float floorVarianceFactor;
            };

            Shared<nite::Map> generateMap(Shared<Game::RING::Blueprint> bp, Game::RING::TileSource &temp, int scale);

        }
   }

#endif