#ifndef GAME_ENTITY_BASE_HPP
    #define GAME_ENTITY_BASE_HPP

    #include "../Network/Object.hpp"
    #include "Stat.hpp"

    namespace Game {
        struct EntityBase : Game::NetObject, Game::Stat {
            UInt8 faceDirection;
            String name;
            float walkPushRate;
			void kill();
            void onCreate();
            void draw();
            void entityMove(float angle, float mod, bool holdStance);
			virtual void onDeath(){
				
			}            
        };
    }

#endif 