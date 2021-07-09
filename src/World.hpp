#ifndef GAME_WORLD_HPP
    #define GAME_WORLD_HPP

    #include "Object.hpp"

    namespace Game {
        struct World { 
            Vector<Shared<Object>> objects;
            UInt16 tickrate;
            UInt64 lastStep;
            UInt32 tick;

            void init(int width, int height);

            void step();

            bool add(const Shared<Object> &obj);

            void render();
        };

    }

#endif