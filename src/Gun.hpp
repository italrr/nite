#ifndef GAME_BASE_GUN_HPP
	#define GAME_BASE_GUN_HPP

	#include "Inventory.hpp"
    #include "Entity.hpp"

	namespace ammoType {
		static const unsigned Undefined = 999;
		static const unsigned HANDGUN9mm = 0;
	}	      	    

	namespace Game {
		struct Gun : public Game::BaseEquip {
			unsigned animNormal;
			nite::Vec2 origin;
            nite::Vec2 barrelPosition;
		    int magSize; 
            int currentLoad;
		    unsigned ammoType;        
			void loadSpecific(const String &path);
            virtual void onShot(Game::Entity *carrier){

            }

		};		
	}

#endif
