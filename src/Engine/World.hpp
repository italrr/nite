#ifndef GAME_WORLD_HPP
	#define GAME_WORLD_HPP

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
			void updateObjectPhysics(Shared<nite::PhysicsObject> obj, float x, float y);
			void remove(int objectId);
			void remove(Shared<nite::PhysicsObject> obj);
			void update();
			void step();
			void render();
		};
	}


#endif
