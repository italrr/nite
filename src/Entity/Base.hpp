#ifndef GAME_ENTITY_BASE_HPP
    #define GAME_ENTITY_BASE_HPP
    
    #include "../AI/AI.hpp"
    #include "../Core/Object.hpp"
    #include "../Core/Input.hpp"
    
    #include "Stat.hpp"
    #include "Anim.hpp"

    #include "../Engine/UI/PanelUI.hpp"
    namespace Game {

        namespace ActionableType {
            enum ActionableType : UInt8 {
                None = 0,
                Skill,
                Item
            };
        }

        static const UInt8 EntityActionables = 7;
        static const UInt64 RecalculateStatTimeout = 150; 

        struct Actionable {
            UInt8 type;
            UInt32 id; // either slotId or skill id
            Actionable(){
                this->id = 0; // an item or skill cannot be 0
                this->type = Game::ActionableType::None;
            }
        };
        namespace EntityStateSlot {
            enum EntityStateSlot : UInt8 {
                MID = 0,
                BOTTOM
            };  
            static const UInt8 total = 2;
        }
        namespace EntityState {
            enum EntityState : UInt8 {
                IDLE = 0, // aka standing
                WALKING,
                JUMPING,
                SHOOTING_HANDGUN,
                SHOOTING_BOW,
                IDLE_HANDGUN,
                IDLE_BOW,
                IDLE_SWORD,
                IDLE_KNIFE,
                WAVING_KNIFE,
                WAVING_SWORD,
                MELEE_NOWEAP, // aka punching
                CASTING,
                IDLE_FIST
            };
            static const UInt8 total = 14;
            // top is not resolved here. we're gonna hard code it for now
            static const UInt8 stateToAnimType[total][EntityStateSlot::total] = {
                // MID                              BOTTOM
                { AnimType::MID_STANDING,           AnimType::BOT_STANDING  },    // IDLE
                { AnimType::MID_STANDING,           AnimType::BOT_WALKING   },    // WALKING
                { AnimType::MID_STANDING,           AnimType::BOT_JUMPING   },    // JUMPING
                { AnimType::MID_SHOOTING_HANDGUN,   AnimType::BOT_STANDING  },    // SHOOTING_HANDGUN
                { AnimType::MID_SHOOTING_BOW,       AnimType::BOT_STANDING  },    // SHOOTING_BOW
                { AnimType::MID_HOLDING_HANDGUN,    AnimType::BOT_STANDING  },    // IDLE_HANDGUN
                { AnimType::MID_HOLDING_BOW,        AnimType::BOT_STANDING  },    // IDLE_BOW
                { AnimType::MID_HOLDING_SWORD,      AnimType::BOT_STANDING  },    // IDLE_SWORD
                { AnimType::MID_HOLDING_KNIFE,      AnimType::BOT_STANDING  },    // IDLE_KNIFE
                { AnimType::MID_WAVING_KNIFE,       AnimType::BOT_STANDING  },    // WAVING_KNIFE
                { AnimType::MID_WAVING_SWORD,       AnimType::BOT_STANDING  },    // WAVING_SWORD
                { AnimType::MID_MELEE_NOWEAP,       AnimType::BOT_STANDING  },    // MELEE_NOWEAP                
                { AnimType::MID_CASTING,            AnimType::BOT_STANDING  },    // CASTING
                { AnimType::MID_MELEE_NOWEAP,       AnimType::BOT_STANDING  }     // IDLE_FISTS
                
            };
        }

        struct EntityCasting {
            UInt16 target;
            UInt16 id;
            UInt8 type;
            UInt64 startTime;
            UInt64 time;
            nite::Vec2 p;
            EntityCasting(){
                startTime = nite::getTicks();
            }
        };

        struct Gfx_CastingBall {
            nite::Vec2 pos;
            nite::Vec2 rPos;
            nite::Texture tex;
            float z;
            float rZ;
            nite::Shader shader;
            float alpha;
            float step;
            void init(const nite::Vec2 &p);
            void draw(const nite::Vec2 &p);
        };

        struct EntityBase;
        struct AIDriver {
            Game::EntityBase *subject;
            Vector<Shared<Game::AI::BaseBehavior>> behaviors;
            void set(Game::EntityBase *subject);
            void update();
            void add(Shared<Game::AI::BaseBehavior> behavior);
        };

        struct EntityBase : Game::NetObject, Game::Stat {

            void addDamageCountShow(int n);
            unsigned dmgCountShow;
            float dmgCountShowAngle;
            float dmgCountShowAlpha;
            nite::Vec2 dmgCountShowPos;
            UInt64 lastDmgCountShow;

            float entityAlpha;
            bool canDamage();
            UInt64 lastDmgd;
            UInt64 lastMeleeHit;
            Game::InputCompacter input;
            EntityBase();
            Game::AIDriver aidriver;
            Shared<Game::EntityCasting> currentCasting;
            UInt8 state[EntityStateSlot::total];
            UInt8 stNum[EntityStateSlot::total];
            UInt64 lastStateTime[EntityStateSlot::total];
            UInt64 lastFrameTime[EntityStateSlot::total];
            void setState(UInt8 nstate, UInt8 slot, UInt8 n);
            void switchFrame(UInt8 slot, UInt8 n);
            Game::Actionable actionables[Game::EntityActionables]; // 1 2 3 4 5 keys + M1 + M2
            UInt8 faceDirection;
            Game::Anim anim;
            bool isMoving;
            bool isCasting;
            String name;
            int walkStepTick;
            bool dead;
            float castingMsgAlpha;
            Gfx_CastingBall castingBall;
            Shared<nite::BaseUIComponent> castingMsg;
            Vector<nite::Hitbox> getHitbox();
            void throwMelee(float x, float y); // time recommended to be not more than 100ms
			void kill();
            void onCreate();
            void draw();
            void solveCasting();
            void entityMove(const nite::Vec2 &dir, bool holdStance);
            void loadAnim();
            void entityStep();
            void printInfo();
            bool damage(const Game::DamageInfo &dmg);
            void markDamaged();
            void invokeUse(UInt16 targetId, UInt8 type, UInt32 id, float x, float y);
            UInt64 lastUpdateStats;
            void recalculateStats();
            virtual void updateStance();
			virtual void onDeath(){
				
			}   

            // stats         
            virtual void writeInitialState(nite::Packet &packet);
            virtual void readInitialState(nite::Packet &packet);     
            virtual void writeAllStatState(nite::Packet &packet);       
            virtual void readAllStatState(nite::Packet &packet); 

            virtual void writeHealthStatState(nite::Packet &packet);       
            virtual void readHealthStatState(nite::Packet &packet);           
        };
    }

#endif 