#ifndef GAME_ENTITY_HPP
  	#define GAME_ENTITY_HPP

	#define GAME_MAX_LEVEL 48
	#define GAME_MAX_STAT 100
	#define GAME_MAX_ATK_RATE GAME_MAX_STAT * (3.85f + GAME_MAX_LEVEL * 0.15f) + GAME_MAX_STAT * (1.1f + GAME_MAX_LEVEL * 0.1f)

  	#include "Engine/Tools/Tools.hpp"
  	#include "Network/Object.hpp"
  	#include "Engine/Texture.hpp"
  	#include "Engine/Animation.hpp"  
  	#include "Inventory.hpp"

	namespace Game {

		namespace EntityStance {
			static const UInt8 Undefined = 255;
			static const UInt8 Neutral = 1;
			static const UInt8 Melee = 2;
			static const UInt8 KnockedBack = 3;
			static const UInt8 Gun = 4;
			static const UInt8 Cast = 5;
			static const UInt8 Bow = 6;
		}

		struct HealthStat {
			UInt32 health; 
			UInt32 maxHealth;
			UInt32 mana;
			UInt32 maxMana;
			UInt32 stamina;
			UInt32 maxStamina;
			UInt32 exp;
			UInt32 currentExp;
			UInt32 nextExp;
			UInt32 lv;
		};

		namespace BaseStatType {
			static const UInt8 Strength = 0;
			static const UInt8 Agility = 1;
			static const UInt8 Dexterity = 2;
			static const UInt8 Endurance = 3;
			static const UInt8 Luck = 4;
			static const UInt8 Intelligence = 5;
			static const UInt8 Charisma = 6;
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
			UInt32 strStat;
			UInt32 agiStat;
			UInt32 dexStat;
			UInt32 enduStat;
			UInt32 lukStat;
			UInt32 intStat;
			UInt32 charismaStat;
			UInt32 statPoints;
			BaseStat();
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
			static const UInt8 Right = 0;
			static const UInt8 Left = 1;
		};

		struct FrameData {
			nite::Vec2 index;
			UInt32 n;
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
			nite::Vec2 gunOnHandPosition;
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
			FrameData topGunShoot;
			FrameData topGunOnHand;
			FrameData topCast;
			// Frame Animation
			FrameAnimation faSwordForward;
			FrameAnimation faSwordOnHand;
			FrameAnimation faGunShoot;
			FrameAnimation faGunOnHand;			
			void update();
			UInt64 lastFileCheck;
			String lastHash;
			String filePath;
		};
		
		struct DamageInfo;
		struct MeleeHitInformation;
		struct Entity : Game::NetObject, HealthStat, BaseStat, ComplexStat, Inventory {
			// Animations
			UInt16 animTopIdle;
			UInt16 animTopWalking;
			UInt16 animTopGunShoot;
			UInt16 animTopGunOnHand;
			UInt16 animTopCast;
			UInt16 animTopSwordSwinging;
			UInt16 animTopSwordOnHand;
			UInt16 animTopKnockback;
			UInt16 animBottomKnockback;
			UInt16 animBottomIdle;
			UInt16 animBottomWalking;
			UInt16 animCurrentTop;
			UInt16 animCurrentBottom;
			
			// Walking Mechanic
			UInt16 walkingLastStep;
			UInt64 walkingTimeout;
			UInt64 walkingStepTimeout;
			bool isWalking;

			// Knockback Mechanic
			UInt64 knockbackTimeout;
			bool isKnockedback;

			// Sword Swing Mechanic
			UInt16 swordSwingLastStep;
			UInt64 swordSwingTimeout;
			bool isSwordSwinging;

			// Gun Shooting Mechanic
			UInt16 gunShootLastStep;
			UInt64 gunShootTimeout;
			bool isShootingGun;	

			// Cast Mechanic	
			UInt16 castLastStep;
			UInt64 castTimeout;
			bool isCasting;					
			
			// Etc
			AnimationData dims;
			nite::Texture entityShadow;
			nite::Animation entityAnim;
			nite::Texture gunShootBang;
			nite::Batch batch;
			UInt8 faceDirection;
			float baseScaleEntity;
			bool dead;
			float entityAlpha;
			float walkPushRate;
			String name;
			UInt8 currentStance;
			UInt8 lastStance;
			void switchStance(UInt8 stance);
			void reverseStance();
			void entityReloadAnimation();
			void entityInit();
			void entityStep();
			void entityUseActiveSlot(UInt16 slotId);
			void onActiveItemSwitch(Shared<Game::BaseItem> &item);
			void recalculateHealthStats();
			void recalculateComplexStats();
			void printEntity();
			void recalculateStats();
			void resetStat(UInt8 type);
			void knockback(float angle, float intensity);
			Int32 addBaseStat(UInt8 type, UInt32 amnt);
			nite::Hitbox getMeleeHitbox();
			void dealDamage(const Game::DamageInfo &dmg);
			void throwMelee(Game::BaseEquip *_weap);
			void shootBullet(Game::BaseEquip *_weap);
			Game::MeleeHitInformation receiveMelee(nite::Hitbox &input);
			void kill();
			void setupEntity(UInt32 lv, float baseScale);
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
			Game::Entity *subject;
			Game::Entity *owner;
			Int32 amount;
			UInt8 element;
			bool knockback;
		};

  	}

#endif 