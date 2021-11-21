#ifndef GAME_WORLD_HPP
    #define GAME_WORLD_HPP

    #include "Engine/View.hpp"
    #include "Engine/Map.hpp"
    #include "Object.hpp"


    namespace Game {

        struct World { 
            Dict<String, Shared<Game::Object>> objects;
            Vector<String> removeQueue;
            String followObject;
            bool useCameraFollow;
            nite::Vec2 cameraFollowNewPos;
            void setFollowObject(bool v, const String &followObject = "");

            UInt16 tickrate;
            UInt64 lastStep;
            UInt32 tick;

            Shared<nite::Map> currentMap;
            Vector<Shared<Game::Object>> wallMasks;
            void useMap(const Shared<nite::Map> &map);
            void resetMap();

            void init(int width, int height);

            void step();

            bool add(const Shared<Object> &obj);
            bool remove(const String &symId);
            bool remove(int refId);
            
            bool isToRemove(const String &symId);
            

            void render();
        };

    }

#endif