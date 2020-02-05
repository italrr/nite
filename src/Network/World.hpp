#ifndef GAME_WORLD_HPP
	#define GAME_WORLD_HPP

	#include <unordered_map>
	#include "Object.hpp"

	namespace Game {
		struct NetWorld {
			std::unordered_map<UInt32, Shared<Game::NetObject>> objects;			
			Vector<UInt32> removeQueue;
			NetWorld();
			float timescale;
			bool debugPhysics;
			void clear();
			bool exists(UInt32 id);
			UInt32 add(Shared<Game::NetObject> obj);
			void remove(UInt32 objectId);
			void remove(Shared<Game::NetObject> obj);
			void update();
			void step();
			void render();
		};
	}


#endif
