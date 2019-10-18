#ifndef GAME_ENTITY_HPP
  	#define GAME_ENTITY_HPP

	#define GAME_MAX_LEVEL 48
	#define GAME_MAX_STAT 100
	#define GAME_MAX_ATK_RATE GAME_MAX_STAT * (3.85f + GAME_MAX_LEVEL * 0.15f) + GAME_MAX_STAT * (1.1f + GAME_MAX_LEVEL * 0.1f)

  	#include "Engine/Tools/Tools.hpp"
  	#include "Engine/Object.hpp"
  	#include "Engine/Texture.hpp"
  	#include "Engine/Animation.hpp"  
  	#include "Inventory.hpp"
  
	namespace Game {

		namespace EntityStance {
			static const unsigned Undefined = 999;
			static const unsigned Neutral = 1;
			static const unsigned Melee = 2;
			static const unsigned KnockedBack = 3;
		}

		struct HealthStat {
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
		};

		namespace BaseStatType {
			static const unsigned Strength = 0;
			static const unsigned Agility = 1;
			static const unsigned Dexterity = 2;
			static const unsigned Endurance = 3;
			static const unsigned Luck = 4;
			static const unsigned Intelligence = 5;
			static const unsigned Charisma = 6;
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

		struct BaseStat {
			int strStat;
			int agiStat;
			int dexStat;
			int enduStat;
			int lukStat;
			int intStat;
			int charismaStat;
			int statPoints;
			BaseStat();
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
			ComplexStat();
		};

		namespace EntityFacing {
			static const unsigned Right = 0;
			static const unsigned Left = 1;
		};

		struct FrameData {
			nite::Vec2 index;
			int n;
			bool reversed;
			bool vertical;
			float speed;
			void parse(const Jzon::Node &node);
		};

		struct FrameAnimationData {
			nite::Vec2 position;
			float angle;
			UInt64 time;
			bool damage;
		};

		struct FrameAnimation {
				Vector<FrameAnimationData> frames;
				void load(const Jzon::Node &node);
				void clear();
		};

		struct Entity;
		struct AnimationData {
			Game::Entity *host;
			Vector<nite::Hitbox> hitboxes;
			nite::Vec2 headPosition;
			nite::Vec2 headSize;
			nite::Vec2 frameSize;
			nite::Vec2 spriteSize;
			nite::Vec2 defaultPhysicalSize;
			float frameDepthOffset;
			float faceDirBias;
			float globalDepthOffsetY;
			float walkRightStepOffset;
			String spriteFilename;
			bool load(const String &path);
			void clear();
			FrameData topIdle;
			FrameData bottomIdle;
			FrameData topWalking;
			FrameData bottomWalking;
			FrameData topKnockback;
			FrameData bottomKnockback;
			FrameData topSwordSwing;
			FrameData topSwordOnHand;
			// Frame Animation
			FrameAnimation faSwordForward;
			FrameAnimation faSwordOnHand;
			void update();
			UInt64 lastFileCheck;
			String lastHash;
			String filePath;
		};
		
		struct DamageInfo;
		struct MeleeHitInformation;
		struct Entity : nite::PhysicsObject, HealthStat, BaseStat, ComplexStat, Inventory {
			// Animations
			unsigned animTopIdle;
			unsigned animTopWalking;
			unsigned animTopSwordSwinging;
			unsigned animTopSwordOnHand;
			unsigned animTopKnockback;
			unsigned animBottomKnockback;
			unsigned animBottomIdle;
			unsigned animBottomWalking;
			unsigned animCurrentTop;
			unsigned animCurrentBottom;
			
			// Walking Mechanic
			int walkingLastStep;
			UInt64 walkingTimeout;
			UInt64 walkingStepTimeout;
			bool isWalking;

			// Knockback Mechanic
			UInt64 knockbackTimeout;
			bool isKnockedback;

			// Sword Swing Mechanic
			int swordSwingLastStep;
			UInt64 swordSwingTimeout;
			bool isSwordSwinging;
			
			// Etc
			AnimationData dims;
			nite::Texture entityShadow;
			nite::Animation entityAnim;
			nite::Batch batch;
			int faceDirection;
			float baseScaleEntity;
			bool dead;
			float entityAlpha;
			float walkPushRate;
			String name;
			int currentStance;
			int lastStance;
			void switchStance(int stance);
			void reverseStance();
			void entityReloadAnimation();
			void entityInit();
			void entityStep();
			void entityUseActiveSlot(int slotId);
			void onActiveItemSwitch(Shared<Game::BaseItem> &item);
			void recalculateHealthStats();
			void recalculateComplexStats();
			void printEntity();
			void recalculateStats();
			void resetStat(int type);
			void knockback(float angle, float intensity);
			int addBaseStat(int type, int amnt);
			nite::Hitbox getMeleeHitbox();
			void dealDamage(const Game::DamageInfo &dmg);
			void throwMelee();
			Game::MeleeHitInformation receiveMelee(nite::Hitbox &input);
			void kill();
			void setupEntity(int lv, float baseScale);
			void fullHeal();
			bool lvUp();
			void onCreate();
			void onDestroy();
			void entityMove(float angle, float mod, bool holdStance = false);
			void step();
			void draw();
			void init();
			virtual void onDeath(){
				
			}
		};

		struct MeleeHitInformation {
			bool hit;
			nite::Hitbox hitbox;
			Entity *subject;
			MeleeHitInformation(){
				hit = false;
			}
		};

		struct DamageInfo {
			Entity *subject;
			Entity *owner;
			int amount;
			int element;
			bool knockback;
		};

  	}

#endif 