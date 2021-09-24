#ifndef GAME_WORLD_HPP
    #define GAME_WORLD_HPP

    #include "Engine/View.hpp"
    #include "Object.hpp"

    namespace Game {
        struct World { 
            Dict<String, Shared<Object>> objects;
            String followObject;
            bool useCameraFollow;
            nite::Vec2 cameraFollowNewPos;
            void setFollowObject(bool v, const String &followObject = "");

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