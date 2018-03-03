#ifndef GAME_BASE_SWORD
	#define GAME_BASE_SWORD

	#include "Inventory.hpp"

	namespace Game {
		struct Sword : public Game::BaseEquip {
			unsigned animNormal;
			nite::Vec2 origin;
			void loadSpecific(const String &path);
		};		
	}

#endif
