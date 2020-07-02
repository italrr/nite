#ifndef GAME_RING_BLUEPRINT_HPP
    #define GAME_RING_BLUEPRINT_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Graphics.hpp"

    namespace Game {

        namespace RING {

            enum RoomType : UInt8 {
                Normal = 0

            };

            enum CellType : UInt8 {
                Wall = 0,
                Path
            };

            struct Room {
                int x;
                int y;
                int width;
                int height;
                int type;
                String name;
            };

            struct Cell {
                int x;
                int y;
                int i;
                int type;
                void set(int x, int y);
                Cell(int x, int y);
                Cell();
                int index(int width);
            };            
            
            struct Blueprint {
                Vector<Game::RING::Room> rooms;

                int nrooms;
                int width;
                int height;
                int total;
                int totalPath;
                int totalWall;
                int order;
                int start;
                int end;
                String name;
                nite::Batch minimap;
                Game::RING::Cell *grid; // TODO: maybe use a map?

                void generate(int w, int h);
                void generate();
                Blueprint();
                ~Blueprint();
                void clear();
                void generateMinimap();

                // tools
                void fill(int type);
                void fillRectangle(int x, int y, int w, int h, int type);
                void fillBorder(int type);
                void set(int x, int y, int type);
            };
        }

    }


#endif