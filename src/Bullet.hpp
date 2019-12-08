#ifndef GAME_BASE_BULLET
    #define GAME_BASE_BULLET

	#include "Engine/Object.hpp"
    #include "Entity.hpp"

	namespace Game {
		struct BaseBullet : public nite::PhysicsObject {
			Entity *owner;
            float mod;
            float angle;
            int weapDmg;
            int bulletDmg;
            float totalDmg;
            UInt64 lifetime; 
            UInt64 createdAt; 
			void onCreate();
			void onDestroy();
			void step();
			void draw();
			void init();   
            void setup(Game::Entity *owner, float mod, float angle, int weapDmg, int bulletDmg, UInt64 lifetime);
		};		
	}    

#endif