#ifndef GAME_RING_MAP_HPP
    #define GAME_RING_MAP_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Texture.hpp"
    #include "../Engine/Tileset.hpp"
    #include "Blueprint.hpp"

    namespace Game {
        namespace RING {

            struct TileTemplate {
                String name;
                String description;
                String sourcePath;
                nite::Texture source;
                nite::Vec2 sourceSize;
                nite::Vec2 margin;
                nite::Vec2 spacing;
                nite::Vec2 tileSize;
                nite::Color transparency;
                TileTemplate(const String &path);
                TileTemplate();
                void load(const String &path);
                bool isIgnoredForFLoors(const String &key);
                Dict<String, int> mapping;
                String floorDefault;
                Vector<String> floorVariants;
                Vector<String> ignoresForFloor;
                String lastFloorVariant;
                String getFloorVariant(Game::RING::Blueprint &bp);
                float floorVarianceFactor;
            };

            struct Layer {
                int *grid;
                int width;
                int height;
                int depth;
                int total;
                Layer(int width, int height, int depth);
                ~Layer();
                void clear();
                void fill(int val);
            };

            struct Map {
                Game::RING::TileTemplate temp;
                Vector<Game::RING::Layer*> layers;
                Shared<Game::RING::Blueprint> bp;
                int width;
                int height;
                int start;
                nite::Vec2 startPosition;
                nite::Vec2 cellSize;
                int size;
                int scale; // one real unit is x times a unit of the blueprint
                void build(Shared<Game::RING::Blueprint> bp, const Game::RING::TileTemplate &temp, int scale);
                Map();
                ~Map();
                void draw();
            };
        }
    }

#endif