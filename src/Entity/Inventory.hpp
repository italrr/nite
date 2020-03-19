#ifndef GAME_INVENTORY_HPP
  #define GAME_INVENTORY_HPP

	#include "../Engine/Types.hpp"
	#include "../Engine/Texture.hpp"
	#include "../Engine/Animation.hpp"
	#include "../Core/Object.hpp"
	#include "Battle.hpp"
  	#include <memory>

	namespace Game {

		#define GAME_ITEM_MAX_COMPOUND_SLOTS 4
		struct EntityBase;

		// some naming conventions for items:
		// U_ -> Usable
		// A_ -> Armor
		// E_ -> Equip
		// MISC_ -> everything else
		namespace ItemList {
			enum EffectList : UInt16 {
				NONE = 0,

				U_APPLE,



			};
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

		namespace EquipType {
			enum ItemType : UInt8 {
				None = 0,
				Weapon, // 
				Headgear,
				Armor,
				Garment, // Or 'cape'
				Accessory,
				Footwear
			};
		}		


		namespace EquipSlot {
			enum EquipSlot : UInt8 {
                Head,
                Chest,
                Body, 
                Footwear,
                LeftHand,
                RightHand,
                LeftAcc,
                RightAcc
			};
			static const UInt8 TOTAL = 8;
		}	

		struct ItemBase {
			UInt16 id;
			UInt16 slotId;
			UInt8 type;
			String name;
			String desc;
			UInt16 weight; // 1 unit = 1 gram
			Int32 qty;
			UInt16 elemnt;
			bool amnt;
			UInt16 compound[GAME_ITEM_MAX_COMPOUND_SLOTS]; // item ids
			ItemBase(){
				id = ItemList::NONE;
				slotId = 0; // it can never be 0
				for(int i = 0; i < GAME_ITEM_MAX_COMPOUND_SLOTS; ++i){
					this->compound[i] = 0; // means no compound
				}
			}
			
            virtual void buildDesc(EntityBase *owner){
                this->desc = "None";
            }

			virtual void use(EntityBase *on, EntityBase *owner){

			}
			
			virtual void onAfterUse(EntityBase *on, EntityBase *owner){

			}
			
			virtual void onBeforeUse(EntityBase *on, EntityBase *owner){

			}
			
			virtual void onCarryAdd(EntityBase *owner){

			}

			virtual void onCarryRemove(EntityBase *owner){

			}
		};

		struct EquipItem : ItemBase {
			UInt8 equipType;
			UInt16 dmg;
			UInt16 mdmg;
			UInt16 def;
			UInt16 mdef;
			virtual void onEquip(EntityBase *owner){
				
			}
			virtual void onUnequip(EntityBase *owner){

			}
			virtual UInt16 onDamageRecv(UInt16 amnt, const Game::DamageInfo &dmg){
				return amnt;
			}
			virtual UInt16 onDamageDone(UInt16 amnt, const Game::DamageInfo &dmg){
				return amnt;
			}
		};

		Shared<Game::ItemBase> getItem(UInt16 id, UInt16 qty);

		struct InventoryStat {
			EntityBase *owner;
			UInt16 seedIndex;
			Shared<ItemBase> slots[EquipSlot::TOTAL];
			Dict<UInt16, Shared<Game::ItemBase>> carry;
			InventoryStat();
			UInt16 add(Shared<Game::ItemBase> &item);
			UInt16 add(Shared<Game::ItemBase> &item, UInt16 slotId);
			Shared<Game::ItemBase> get(UInt16 slotId);
			bool contains(UInt16 id);
			bool equip(Shared<Game::ItemBase> &item);
			bool unequip(UInt16 itemId);
			void clear();
			bool remove(UInt16 id, UInt16 qty);
			bool removeBySlotId(UInt16 slotId, UInt16 qty);	
			// TODO: add use	
		};

		namespace Items {
			struct HealthPotion : ItemBase {
				HealthPotion(){
					id = ItemList::U_APPLE;
					name = "Apple";
					desc = "Heals 25 HP points over 5 seconds";
					type = ItemType::Usable;
					weight = 100;
				}
			};
		}
	}

#endif
