#ifndef GAME_WORLD_HPP
	#define GAME_WORLD_HPP

	#include <unordered_map>
	#include "Object.hpp"

	namespace Game {

		namespace ScopePriority {
			enum ScopePriority : int {
				LOW,
				MID,
				HIGH,
				IMMEDIATE
			};
		}


		struct WorldScope {
			int x;
			int y;
			int w;
			int h;
			int priority;
		};


		struct GhostMask : Game::NetObject {
			GhostMask(){
				callback = [](Game::NetObject *subj){
					return;
				};
				solid = false;
				unmovable = true;
				this->objType = ObjectType::Ghost;
			}
			std::function<void(Game::NetObject *subj)> callback;
		};

		struct NetWorld {
			UInt16 generateId();
			UInt16 seedId;
			int seedNId;
			std::unordered_map<UInt16, Shared<Game::NetObject>> objects;	
			Vector<Game::NetObject*> wallMasks;
			Vector<Game::NetObject*> ghostMasks;
			Dict<UInt16, Game::NetObject*> updateQueue;	
			Vector<UInt16> removeQueue;
			UInt64 tickrate;
			float currentTickRate;
			UInt64 delta;
			UInt64 lastTick;
			NetWorld();
			float timescale;
			bool debugPhysics;
			UInt32 snapshotOrder;
			void clear();
			bool exists(UInt16 id);
			void updateObject(Game::NetObject *obj);
			UInt16 add(Shared<Game::NetObject> &obj, int useId = -1);
			bool testCollision(Game::NetObject *a, Game::NetObject *b, const nite::Vec2 &diff, nite::Vec2 &limit, nite::Vec2 &normal);
			void remove(int objectId);
			void remove(Game::NetObject *obj);
			Game::NetObject *get(UInt16 id);
			void update();
			void step();
			void render();
			nite::Vec2 gridSpec; // gridspec for grid positioning
			// cell grid is smaller (or should be)
			void clearWallMasks();
			int addWallMask(Game::NetObject *mask);
			bool removeWallMask(Game::NetObject *mask);
			void clearGhostMasks();
			int addGhostMask(Game::NetObject *mask);
			bool removeGhostMask(Game::NetObject *mask);
			Game::NetObject **cells;
			Game::NetObject **ghosts;
			int ctotal;
			int cwidth;
			int cheight;
			int cusize;
			nite::Vec2 size;
			void setSize(int w, int h, int unitsize, float gridSpec);
			void getQuadrant(int x, int y, int w, int h, Vector<Game::NetObject*> &holder);
			void setVisibleQuadrant(int x, int y, int w, int h);
			~NetWorld();
		};
	}


#endif
