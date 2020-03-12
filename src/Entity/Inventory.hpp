#ifndef GAME_INVENTORY_HPP
  #define GAME_INVENTORY_HPP

	#include "../Engine/Types.hpp"
	#include "../Engine/Texture.hpp"
	#include "../Engine/Animation.hpp"
	#include "../Core/Object.hpp"
  	#include <memory>

	namespace Game {

		namespace Element {
			enum Element : UInt8 {
				None = 0,
				Neutral,
				Fire,
				Electric,
				Steel,
				Wind,
				Umbra,
				Celestial,
				Emerium
			};
			static String name(int id){
				switch(id){
					case Neutral:
						return "Neutral";
					case Fire:
						return "Fire";
					case Electric:
						return "Electric";
					case Umbra:
						return "Umbra";	
					case Celestial:
						return "Celestial";							
					case Steel:
						return "Steel";			
					case Emerium:
						return "Emerium";			
					default:
						return "None";																										
				}
			}
		}

		namespace ItemType {
			enum ItemType : UInt8 {
				None = 0,
				Equip, // weapons
				Armor, // armor, shield, footwear, etc
				Usable, // potions, scrolls
				Etc // everything else
			};
		}

		struct BaseItem {
			UInt16 id;
			UInt16 slotId;
			UInt8 type;
			String name;
			String desc;
			UInt16 weight;
			Int32 qty;
			UInt16 elemnt;
			bool amnt;

			BaseItem(){
				slotId = 0; // it can never be 0
			}
			
			virtual void use(){

			}
			
			virtual void onAfterUse(){

			}
			
			virtual void onBeforeUse(){

			}
			
			virtual void onCarryAdd(){

			}

			virtual void onCarryRemove(){

			}
		};

		struct InventoryStat {
			UInt16 seedIndex;
			Dict<UInt16, Shared<Game::BaseItem>> carry;
			InventoryStat();
			UInt16 add(Shared<Game::BaseItem> &item);
			Shared<Game::BaseItem> get(UInt16 itemId);
			bool contains(UInt16 id);
			void clear();
			bool remove(UInt16 id, UInt16 amnt);		
		};
	}

#endif
