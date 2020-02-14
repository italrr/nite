#ifndef GAME_WORLD_HPP
	#define GAME_WORLD_HPP

	#include <unordered_map>
	#include "Object.hpp"

	namespace Game {
		struct NetWorld {
			std::unordered_map<UInt16, Shared<Game::NetObject>> objects;	
			Vector<UInt16> updateQueue;	
			Vector<UInt16> removeQueue;
			NetWorld();
			float timescale;
			bool debugPhysics;
			void clear();
			bool exists(UInt16 id);
			void updateObjectPhysics(Shared<nite::PhysicsObject> obj, float x, float y);
			UInt16 add(Shared<Game::NetObject> obj);
			void remove(UInt16 objectId);
			void remove(Shared<Game::NetObject> obj);
			void update();
			void step();
			void render();
		};
	}


#endif
