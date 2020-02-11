#ifndef GAME_PLAYER_HPP
    #define GAME_PLAYER_HPP

    #include "Entity.hpp"

    namespace Game {
        struct Player : public Entity {
            UInt16 startLv;
            void onCreate();
            void step();
            Player(){
                this->startLv = 1;
            }
        };
    }

#endif