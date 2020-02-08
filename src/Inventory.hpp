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
			Undefined = 0,
			Neutral,
			Fire,
			Electric,
			Umbra,
			Light,
			Steel,
			Emerium,
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
				case Light:
					return "Light";							
				case Steel:
					return "Steel";			
				case Emerium:
					return "Emerium";			
				default:
					return "Undefined";																										
			}
		}
	}

	namespace ItemType {
		enum ItemType : UInt8 {
			Undefined = 0,
			Equip		
		};
	}

	namespace EquipType {
		enum EquipType : UInt8 {
			Undefined = 0,
			Sword,
			Gun
		};
	}	

	namespace ItemEffectType {
		enum ItemEffectType : UInt8 {
			Undefined = 0
		};
	}

	namespace InventoryActiveSlot {
		enum InventoryActiveSlot : UInt8 {
			Undefined = 0,
			Main,
			Secondary
		};
	}	

	struct ItemEffect {
		int bias;
		float amount;
	};

	struct Inventory;
	struct BaseItem {
		int inCarryId;
		Inventory *container;
		int id;
		int type;
		String name;
		String description;
		int weight;
		int quantity;
		nite::Texture icon;
		nite::Vec2 spriteSize;
		nite::Vec2 frameSize;
		bool amountable;
		Vector<nite::Hitbox> hitboxes;	
		// BaseItem();
		// BaseItem(const String &path);
		void load(const String &path);
		
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

	struct BaseEquip : public Game::BaseItem {
		nite::Animation equipAnim;
		nite::Texture equipTexture;
		nite::Hitbox correctHitbox(nite::Hitbox &hb, float angle, nite::Vec2 &origin);
		int equipType;
		float damage;
		float defense;
		float lifespan;
		float handling;
		virtual int getEnchant(){
			return Game::Element::Neutral;
		}
		virtual void onEquip(){

		}

		virtual void onUnequip(){

		}

	};

	struct Inventory {
		int carrySeedIndex;
		Dict<int, int> itemHandling;
		Dict<int, Shared<Game::BaseItem>> itemCarry;
		virtual void onActiveItemSwitch(Shared<Game::BaseItem> &item){}
		void setActiveItem(int slotId, Shared<Game::BaseItem> &item);
		void setUnactiveItem(int slotId);
		void setUnactiveItem(Shared<Game::BaseItem> &item);
		Inventory();
		int addItem(Shared<Game::BaseItem> &item);
		Shared<Game::BaseItem> getItem(int itemId);
		bool containsItem(int id);
		bool removeItem(int id);
	};
  }

#endif
