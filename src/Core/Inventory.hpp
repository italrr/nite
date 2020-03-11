#ifndef GAME_INVENTORY_HPP
  #define GAME_INVENTORY_HPP

	#include "Engine/Types.hpp"
	#include "Engine/Texture.hpp"
	#include "Engine/Animation.hpp"
	#include "Engine/Object.hpp"
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
			Undefined = 0,
			Equip,
			Consumable,
			Etc,
			Chestwar,
			Footwear,
			Headwear,
			RightAcc,
			LeftAcc
		};
	}

	struct Inventory;
	struct BaseItem {
		int id;
		int type;
		String name;
		String description;
		int weight;
		int quantity;
		bool amountable;
		
		virtual void loadSpecific(const String &path){

		}
		
		virtual void use(){

		}
		
		virtual void afterUse(){

		}
		
		virtual void beforeUse(){

		}
		
		virtual void onCarryAdd(){

		}

		virtual void onCarryRemove(){

		}
	};
  }

#endif
