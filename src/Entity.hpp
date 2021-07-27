#ifndef GAME_ENTITY_HPP
    #define GAME_ENTITY_HPP

    #include "Object.hpp"

	#define GAME_MAX_LEVEL 48
	#define GAME_MAX_STAT 100
	#define GAME_MAX_ATK_RATE GAME_MAX_STAT * (3.85f + GAME_MAX_LEVEL * 0.15f) + GAME_MAX_STAT * (1.1f + GAME_MAX_LEVEL * 0.1f)
    #define GAME_STAT_BASE_SCALE 8.0f
	#define GAME_STAT_POINTS_PER_LV 6

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
			enum EntityType : int {
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
			enum EntityClass : int {
				Undefined = 0,
				Player,
				Mob,
				NPC,
				Boss,
				MiniBoss
			};
		}

		namespace BaseStatType {
			enum BaseStatType : int {
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
					default: 
						return "Undefined";
				}
			}
		}		

        
        struct BaseStat {
			int str;
			int agi;
			int dex;
			int endu;
			int luk;
			int intel;
			int charm;


			int strAdd;
			int agiAdd;
			int dexAdd;
			int enduAdd;
			int lukAdd;
			int intelAdd;
			int charmAdd;

			int statPoints;
			BaseStat();
			void resetAdd();            
        };

        struct HealthStat {
            bool dead;
			int health; 
			int maxHealth;
			int mana;
			int maxMana;
			int stamina;
			int maxStamina;
			int exp;
			int currentExp;
			int nextExp;
			int lv;
			HealthStat(){
				health = 0;
				maxHealth = 0;
				mana = 0;
				maxMana = 0;
				stamina = 0;
				maxStamina = 0;
			}            
        };

        struct ComplexStat {
			int carry;
			int maxCarry;
			int atk;
			int magicAtk;
			int def;
			int magicDef;
			float critRate;
			float atkRate;
			float walkRate; // 'steps' per second
			float precsRate;
			float persuasionRate;
			float charmRate;
			float cooldownRedRate;
            ComplexStat();
        };

        struct EntityStat {
			Game::HealthStat healthStat;
			Game::BaseStat baseStat;
			Game::ComplexStat complexStat;
			void recalculateBaseStats();
			void recalculateHealthStats();
			void recalculateComplexStats();
			void normalizeComplexStats();
            Int32 addBaseStat(int type, int amnt);
            virtual void recalculateStats();
			void resetBaseStat(int type);
			virtual void resetComplexStats();
			void fullHeal();
			bool heal(int hp, int mana, int stamina);
			bool lvUp();
			bool lvUp(int lvs);
            void setupStat(int lv);            
        };

        struct Entity : Object, EntityStat {

        
        };


    }


#endif