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

            struct TileSource {
                String name;
                String description;
                String sourcePath;
                nite::Texture source;
                nite::Vec2 sourceSize;
                nite::Vec2 margin;
                nite::Vec2 spacing;
                nite::Vec2 tileSize;
                nite::Color transparency;
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