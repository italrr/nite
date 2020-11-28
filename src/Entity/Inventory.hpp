#ifndef GAME_INVENTORY_HPP
  #define GAME_INVENTORY_HPP

	#include "../Engine/Types.hpp"
	#include "../Engine/Texture.hpp"
	#include "../Engine/Animation.hpp"
	#include "../Engine/Indexer.hpp"
	#include "../Core/Object.hpp"
	#include "Battle.hpp"
  	#include <memory>

	namespace Game {

		#define GAME_ITEM_MAX_COMPOUND_SLOTS 4
		#define GAME_HANDS_MAX_NUMBER 2
		struct EntityBase;

		// some naming conventions for items:
		// U_ -> Usable
		// A_ -> Armor
		// E_ -> Equip
		// MISC_ -> everything else
		namespace ItemList {
			enum EffectList : UInt16 {
				NONE = 0,
				NONE_EQUIP = 1,
				U_APPLE,
				W_BOW



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
			static String name(UInt8 v){
				switch(v){
					case EquipSlot::Head: {
						return "Head";
					} break;
					case EquipSlot::Chest: {
						return "Chest";
					} break;
					case EquipSlot::Body: {
						return "Body";
					} break;
					case EquipSlot::Footwear: {
						return "Footwear";
					} break;
					case EquipSlot::LeftHand: {
						return "LeftHand";
					} break;
					case EquipSlot::RightHand: {
						return "RightHand";
					} break;	
					case EquipSlot::LeftAcc: {
						return "LeftAcc";
					} break;
					case EquipSlot::RightAcc: {
						return "RightAcc";
					} break;																																			
					default: {
						return "Undefined";
					} break;
				}
			}
			static const UInt8 TOTAL = 8;
		}	

		namespace WeaponType {
			enum WeaponType : UInt8 {
				Bow,
				Crossbow,
				Sword,
				Knife,
				Katar,
				None
			};
			static const UInt8 TOTAL = 5;
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
			UInt8 upgradelv;
			bool amnt;
			UInt16 compound[GAME_ITEM_MAX_COMPOUND_SLOTS]; // item ids
			int iconId;

			ItemBase(){
				this->upgradelv = 1; // usually for equips
				this->iconId = 0;
				this->id = ItemList::NONE;
				this->slotId = 0; // it can never be 0
				for(int i = 0; i < GAME_ITEM_MAX_COMPOUND_SLOTS; ++i){
					this->compound[i] = 0; // no compounds since an item id cannot be 0
				}
			}
			

			virtual void onRecalculateStat(EntityBase *owner){ // ie passive 

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

			void parse(Jzon::Node &obj);

			virtual void parseSpecial(Jzon::Node &obj){
				
			}
		};

		struct EquipAnimFrame {
			nite::Vec2 inTexCoors;
			String key;
		};

		struct EquipAnim {
			nite::Vec2 textureSize;
			nite::Texture texture;
			nite::IndexedFile source;
			nite::Vec2 frameSize;
			Vector<Game::EquipAnimFrame> frames;
			nite::Vec2 inTexSize;
			nite::Vec2 origin;
			bool loadedTexture;
			nite::Color transparency;
			bool load(const String &path);
			bool loadTexture();
			EquipAnim(){
				transparency = nite::Color("#ffffff");
				loadedTexture = false;	
			}
		};

		struct EquipItem : ItemBase {
			UInt8 equipType;
			UInt8 equipSlot;
			UInt8 weaponType;
			UInt16 dmg;
			UInt16 mdmg;
			UInt16 def;
			UInt16 mdef;
			
			EquipItem(){
				type = ItemType::Equip;	
				weaponType = WeaponType::None;
			}
			
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
			void parseSpecial(Jzon::Node &obj);
		};

		struct EquipWeapon : EquipItem {
			Game::EquipAnim anim;
			EquipWeapon(){
				equipType = EquipType::Weapon;
			}
			void parseSpecial(Jzon::Node &obj);
		}; 

		Shared<Game::ItemBase> getItem(UInt16 id, UInt16 qty);
		void DBLoadInventory(const String &path);

		struct InventoryStat {
			Game::EntityBase *owner;
			UInt16 seedIndex;
			Shared<Game::ItemBase> slots[EquipSlot::TOTAL];
			Dict<UInt16, Shared<Game::ItemBase>> carry;
			Game::EquipWeapon *activeWeapon;
			InventoryStat();
			UInt16 add(Shared<Game::ItemBase> &item);
			UInt16 add(Shared<Game::ItemBase> &item, UInt16 slotId);
			Shared<Game::ItemBase> get(UInt16 slotId);
			Shared<Game::ItemBase> getById(UInt16 slotId);
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
				}
			};
			struct BowWeapon : EquipWeapon {
				BowWeapon(){
					id = ItemList::W_BOW;
				}
			};			
		}
	}

#endif
