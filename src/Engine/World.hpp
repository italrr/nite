#ifndef GAME_WORLD
	#define GAME_WORLD

	#include <unordered_map>
	#include "Object.hpp"

	namespace nite {
		struct World {
			std::unordered_map<int, Shared<nite::PhysicsObject>> objects;			
			Vector<int> removeQueue;
			World();
			float timescale;
			bool debugPhysics;
			void clear();
			bool exists(int id);
			int add(Shared<nite::PhysicsObject> obj);
			void remove(int objectId);
			void remove(Shared<nite::PhysicsObject> obj);
			void update();
			void step();
			void render();
		};
	}


#endif
