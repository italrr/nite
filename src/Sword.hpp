#ifndef GAME_BASE_SWORD
	#define GAME_BASE_SWORD

	#include "Inventory.hpp"
	#include "Entity.hpp"

	namespace Game {
		struct Sword : public Game::BaseEquip {
			unsigned animNormal;
			nite::Vec2 origin;
			void loadSpecific(const String &path);

            virtual void onSwingStart(Game::Entity *carrier){

            }

			virtual void onHit(Game::Entity *carrier, Game::Entity *receiver){

			}

            virtual void onSwingFinish(Game::Entity *carrier){

            }

		};		
	}

#endif
