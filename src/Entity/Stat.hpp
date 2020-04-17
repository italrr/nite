#ifndef GAME_ENTITY_STAT_HPP
    #define GAME_ENTITY_STAT_HPP

    #include "../Engine/Tools/Tools.hpp"
	#include "Effect.hpp"
	#include "Skill.hpp"
	#include "Inventory.hpp"

	#define GAME_MAX_LEVEL 48
	#define GAME_MAX_STAT 100
	#define GAME_MAX_ATK_RATE GAME_MAX_STAT * (3.85f + GAME_MAX_LEVEL * 0.15f) + GAME_MAX_STAT * (1.1f + GAME_MAX_LEVEL * 0.1f)
    #define GAME_STAT_BASE_SCALE 1.0f
	#define GAME_STAT_POINTS_PER_LV 6
	#define GAME_STAT_TYPE UInt16

    namespace Game {

		namespace EntityStance {
			enum EntityStance : UInt8 {
				Undefined = 0,
				Neutral,
				Melee,
				KnockedBack,
				Gun,
				Cast,
				Bow
			};
		}

		namespace EntityType {
			enum EntityType : UInt8 {
				Undefined = 0,
				Humanoid,
				Spectro,
				Demon,
				Angel,
				Plant,
				Insect,
				Fish,
				Mamal,
				Undead
			};
		}
		
		namespace EntityClass {
			enum EntityClass : UInt8 {
				Undefined = 0,
				Player,
				Mob,
				NPC,
				Boss,
				MiniBoss
			};
		}

		namespace BaseStatType {
			enum BaseStatType : UInt8 {
				Strength = 0,
				Agility,
				Dexterity,
				Endurance,
				Luck,
				Intelligence,
				Charisma
			};
			static String name(int id){
				switch(id){
					case Strength: 
						return "Strength";
					case Agility: 
						return "Agility";
					case Dexterity: 
						return "Dexterity";
					case Endurance: 
						return "Endurance";				
					case Luck: 
						return "Luck";			
					case Intelligence: 
						return "Intelligence";
					case Charisma: 
						return "Charisma";																						
				}
			}
		}		

		struct HealthStat {
            bool dead;
			UInt32 health; 
			UInt32 maxHealth;
			UInt32 mana;
			UInt32 maxMana;
			UInt32 stamina;
			UInt32 maxStamina;
			UInt32 exp;
			UInt32 currentExp;
			UInt32 nextExp;
			UInt16 lv;
		};

		struct BaseStat {
			GAME_STAT_TYPE str;
			GAME_STAT_TYPE agi;
			GAME_STAT_TYPE dex;
			GAME_STAT_TYPE endu;
			GAME_STAT_TYPE luk;
			GAME_STAT_TYPE intel;
			GAME_STAT_TYPE charm;

			GAME_STAT_TYPE strAdd;
			GAME_STAT_TYPE agiAdd;
			GAME_STAT_TYPE dexAdd;
			GAME_STAT_TYPE enduAdd;
			GAME_STAT_TYPE lukAdd;
			GAME_STAT_TYPE intelAdd;
			GAME_STAT_TYPE charmAdd;

			GAME_STAT_TYPE statPoints;
			BaseStat();
			void resetAdd();
		};

		struct ComplexStat { 
			UInt32 carry;
			UInt32 maxCarry;
			UInt32 atk;
			UInt32 magicAtk;
			UInt32 def;
			UInt32 magicDef;
			float critRate;
			float atkRate;
			float walkRate; // 'steps' per second
			float precsRate;
			float persuasionRate;
			float charmRate;
			ComplexStat();
		};

		namespace EntityFacing {
            enum EntityFacing : UInt8 {
			    Right = 0,
			    Left,
            };
		}

        struct Stat {
			Game::HealthStat healthStat;
			Game::BaseStat baseStat;
			Game::ComplexStat complexStat;
			Game::EffectStat effectStat;
			Game::SkillStat skillStat;
			Game::InventoryStat invStat;
			void recalculateBaseStats();
			void recalculateHealthStats();
			void recalculateComplexStats();
            Int32 addBaseStat(UInt8 type, GAME_STAT_TYPE amnt);
            virtual void recalculateStats();
			void resetBaseStat(UInt8 type);
			void fullHeal();
			void heal(UInt32 hp, UInt32 mana, UInt32 stamina);
			bool lvUp();       
            void setupStat(UInt16 lv);
        };


    }

#endif

