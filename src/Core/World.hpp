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
			void updateObjectPhysics(Game::NetObject *obj, float x, float y);
			UInt16 add(Shared<Game::NetObject> &obj);
			void remove(UInt16 objectId);
			void remove(Game::NetObject *obj);
			Shared<Game::NetObject> get(UInt16 id);
			void update();
			void step();
			void render();

			
			Game::NetObject **cells; 
			int ctotal;
			int cwidth;
			int cheight;
			int cusize;
			void setSize(int w, int h, int unitsize);
			Vector<Game::NetObject*> getQuadrant(int x, int y, int w, int h);
			~NetWorld();
		};
	}


#endif
