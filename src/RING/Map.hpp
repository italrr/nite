#ifndef GAME_RING_MAP_HPP
    #define GAME_RING_MAP_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Texture.hpp"
    #include "../Engine/Tileset.hpp"
    #include "../Engine/Object.hpp"
    #include "Blueprint.hpp"

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

            struct Layer {
                int *grid;
                int width;
                int height;
                int depth;
                int total;
                bool dynamicY;
                Layer(int width, int height, int depth);
                Layer(int width, int height, int depth, bool dynamicY);
                void set(int width, int height, int depth, bool dynamicY);
                ~Layer();
                void clear();
                void fill(int val);
            };

            struct WallMask {
                nite::Vec2 position;
                nite::Vec2 size;
                bool solid;
                WallMask(const nite::Vec2 &position, const nite::Vec2 &size, bool solid){
                    this->position.set(position);
                    this->size.set(size);
                    this->solid = solid;
                }
                WallMask(){

                }
            };

            struct Map {
                Game::RING::TileSource temp;
                Vector<Game::RING::Layer*> layers;
                Shared<Game::RING::Blueprint> bp;
                Vector<Game::RING::WallMask> wallMasks;

                int width;
                int height;
                int startCell;
                nite::Vec2 startPosition;
                nite::Vec2 cellSize;
                int size;
                int scale; // one real unit is x times a unit of the blueprint
                void build(Shared<Game::RING::Blueprint> bp, const Game::RING::TileSource &temp, int scale);
                Map();
                ~Map();
                void draw();
            };
        }
    }

#endif