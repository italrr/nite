#ifndef GAME_ENTITY_HPP
    #define GAME_ENTITY_HPP
	
	#include "Engine/Texture.hpp"
	#include "Engine/Animation.hpp"
    #include "Object.hpp"

	#define GAME_MAX_LEVEL 48
	#define GAME_MAX_STAT 100
	#define GAME_MAX_ATK_RATE GAME_MAX_STAT * (3.85f + GAME_MAX_LEVEL * 0.15f) + GAME_MAX_STAT * (1.1f + GAME_MAX_LEVEL * 0.1f)
    #define GAME_STAT_BASE_SCALE 1.0f
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
			float luckRate;
			float fleeRate;
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
				ATTACK_MISS,
				STUTTER, // receive damage
				BLOCK,
				BLOCK_STUTTER,
				EVADE,
				EVADE_FAILS,
				DEATH
			};
		}

		struct Entity;
		struct DamageInfo {
			Shared<Entity> owner;
			Shared<Entity> target;
			bool tryingBlock;
			bool tryingDodge;
			int blockDmg;
			int dmg;
			bool dodged;
			bool byLuck;
			int element;
			bool isCrit;
			DamageInfo(){
				dodged = false;
				byLuck = false;
				isCrit = false;
				tryingBlock = false;
				tryingDodge = false;
			}
		};

		namespace OverworldAnimType {
			enum OverworldAnimType {
				NONE,

				STAND_RIGHT,
				STAND_LEFT,
				STAND_DOWN,
				STAND_UP,			
				STAND_UP_RIGHT,				
				STAND_DOWN_RIGHT,
				STAND_UP_LEFT,
				STAND_DOWN_LEFT,

				WALK_RIGHT,
				WALK_LEFT,
				WALK_DOWN,
				WALK_UP,			
				WALK_UP_RIGHT,				
				WALK_DOWN_RIGHT,
				WALK_UP_LEFT,
				WALK_DOWN_LEFT,

				RUN_RIGHT,
				RUN_LEFT,
				RUN_DOWN,
				RUN_UP,			
				RUN_UP_RIGHT,				
				RUN_DOWN_RIGHT,
				RUN_UP_LEFT,
				RUN_DOWN_LEFT,				

				EMOTE_RIGHT,
				EMOTE_LEFT,
				EMOTE_UP,
				EMOTE_DOWN,
				EMOTE_UP_RIGHT,
				EMOTE_DOWN_RIGHT,
				EMOTE_UP_LEFT,
				EMOTE_DOWN_LEFT
			};
			static int type(const String &value){
				
				if(value == "STAND_RIGHT"){
					return STAND_RIGHT;
				}else
				if(value == "STAND_LEFT"){
					return STAND_LEFT;
				}else
				if(value == "STAND_DOWN"){
					return STAND_DOWN;
				}else
				if(value == "STAND_UP"){
					return STAND_UP;
				}else
				if(value == "STAND_UP_RIGHT"){
					return STAND_UP_RIGHT;
				}else
				if(value == "STAND_DOWN_RIGHT"){
					return STAND_DOWN_RIGHT;
				}else
				if(value == "STAND_UP_LEFT"){
					return STAND_UP_LEFT;
				}else
				if(value == "STAND_DOWN_LEFT"){
					return STAND_DOWN_LEFT;
				}else

				if(value == "WALK_RIGHT"){
					return WALK_RIGHT;
				}else
				if(value == "WALK_LEFT"){
					return WALK_LEFT;
				}else
				if(value == "WALK_DOWN"){
					return WALK_DOWN;
				}else
				if(value == "WALK_UP"){
					return WALK_UP;
				}else
				if(value == "WALK_UP_RIGHT"){
					return WALK_UP_RIGHT;
				}else
				if(value == "WALK_DOWN_RIGHT"){
					return WALK_DOWN_RIGHT;
				}else
				if(value == "WALK_UP_LEFT"){
					return WALK_UP_LEFT;
				}else
				if(value == "WALK_DOWN_LEFT"){
					return WALK_DOWN_LEFT;
				}else
				if(value == "RUN_RIGHT"){
					return RUN_RIGHT;
				}else
				if(value == "RUN_LEFT"){
					return RUN_LEFT;
				}else
				if(value == "RUN_DOWN"){
					return RUN_DOWN;
				}else
				if(value == "RUN_UP"){
					return RUN_UP;
				}else
				if(value == "RUN_UP_RIGHT"){
					return RUN_UP_RIGHT;
				}else
				if(value == "RUN_DOWN_RIGHT"){
					return RUN_DOWN_RIGHT;
				}else
				if(value == "RUN_UP_LEFT"){
					return RUN_UP_LEFT;
				}else
				if(value == "RUN_DOWN_LEFT"){
					return RUN_DOWN_LEFT;
				}else
				if(value == "EMOTE_RIGHT"){
					return EMOTE_RIGHT;
				}else
				if(value == "EMOTE_LEFT"){
					return EMOTE_LEFT;
				}else
				if(value == "EMOTE_UP"){
					return EMOTE_UP;
				}else
				if(value == "EMOTE_DOWN"){
					return EMOTE_DOWN;
				}else
				if(value == "EMOTE_UP_RIGHT"){
					return EMOTE_UP_RIGHT;
				}else
				if(value == "EMOTE_DOWN_RIGHT"){
					return EMOTE_DOWN_RIGHT;
				}else
				if(value == "EMOTE_UP_LEFT"){
					return EMOTE_UP_LEFT;
				}else
				if(value == "EMOTE_DOWN_LEFT"){
					return EMOTE_DOWN_LEFT;
				}else{
					return NONE;
				}		
			}
		}

		namespace OverworldAnimEmoteType {
			enum OverworldAnimEmoteType {
				SAYS_HI,
				SAYS_BYE,
				KISS,
				SIGH,
				GRUNT
			};
		}

		namespace EntityFaceDir {
			enum EntityFaceDir {
				RIGHT,
				UP,
				LEFT,
				DOWN,
				UP_RIGHT,
				DOWN_RIGHT,
				UP_LEFT,
				DOWN_LEFT
			};
		}

		struct Entity;
		struct EntityOverworldAnim {
			int id;
			int frameRate; //timeout
		};
		struct EntityOverworld {
			nite::Texture base;
			nite::Animation frames;
			Dict<int, Shared<EntityOverworldAnim>> animations;
			bool load(const String &path);
		};

        struct Entity : Object, EntityStat {
			Shared<EntityOverworld> animOverworld;
			nite::Texture battleAnim;

			int ovwAnim;
			int ovwFrame;
			int faceDir; // 0 = RIGHT
			bool isMoving;
			UInt64 moveTriggerTimeout;

			UInt64 ovwFrameRate;
			UInt64 ovwFrameTick;

			int entityType;
			String nickname;
			float walkSpeed;
			void setAnim(int anim, int frame);
			void updateStandAnim();
			Entity();
			void printInfo();
			void loadAnim();
			void stepAnim();
			void moveEntity(float x, float y);
			bool damage(DamageInfo &info);
			void renderOverworld();
        };


    }


#endif