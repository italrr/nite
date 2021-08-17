#ifndef GAME_ENTITY_HPP
    #define GAME_ENTITY_HPP
	
	#include "Engine/Texture.hpp"
    #include "Object.hpp"

	#define GAME_MAX_LEVEL 48
	#define GAME_MAX_STAT 100
	#define GAME_MAX_ATK_RATE GAME_MAX_STAT * (3.85f + GAME_MAX_LEVEL * 0.15f) + GAME_MAX_STAT * (1.1f + GAME_MAX_LEVEL * 0.1f)
    #define GAME_STAT_BASE_SCALE 8.0f
	#define GAME_STAT_POINTS_PER_LV 6

    namespace Game {

		namespace EntityRace {
			enum EntityRace : int {
				UNDEFINED = 0,
				HUMANOID,
				SPECTER,
				DEMON,
				ANGEL,
				PLANT,
				INSECT,
				FISH,
				MAMAL,
				UNDEAD
			};
		}
		
		namespace EntityType {
			enum EntityType : int {
				UNDEFINED = 0,
				PLAYER,
				MOB,
				NPC,
				BOSS,
				MINIBOSS
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

		namespace EntityBattleAnim {
			enum EntityBattleAnim : int {
				IDLE,
				DEFEAT,
				VICTORY,
				ATTACK, // plain
				STUTTER, // receive damage
				BLOCK,
				BLOCK_STUTTER,
				EVADE,
				EVADE_FAILS,
			};
		}

        struct Entity : Object, EntityStat {
			nite::Texture battleAnim;
			float battleAnimBlink;
			bool battlAnimBlinkFlip;
			UInt64 lastBattleAnimBlinkTick;

			UInt64 battleAnimTargetTime;
			int battleAnimStatus;
			float battleAnimTargetExp;
			UInt64 lastBattleAnimTick;
			int battleAnimStep;
			nite::Vec2 battlAnimPosOff;


			void renderBattleAnim(float x, float y, bool blink);
			void setBattleAnim(int anim, UInt64 animTargetTime);
			bool isBattleAnim();
			int entityType;
			String nickname;
			float walkSpeed;
			int id;
			Entity();
			void loadAnim();
			void moveEntity(float x, float y);
        };


    }


#endif