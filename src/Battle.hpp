#ifndef GAME_BATTLE_HPP
    #define GAME_BATTLE_HPP

    #include "Entity.hpp"
    #include "Dialog.hpp"

    #include "Dialog.hpp"

    namespace Game {

        namespace BattleGroup {
            enum BattleGroup : int {
                GROUP_A,
                GROUP_B
            };
            static String name(int type){
                switch(type){
                    case GROUP_A: {
                        return "GROUP A";
                    } break;
                    case GROUP_B: {
                        return "GROUP B";
                    } break;           
                    default: {
                        return "UNDEFINED";
                    } break;      
                }
            }
        }

        enum BattleState : int {
            BATTLE_READY,
            BATTLE_START,
            PRE_TURN,
            TURN_START,

            PRE_PICK_ACTION,
            PICK_ACTION,
            PICK_ENGAGE_OPT,
            PICK_EVADE_OPT,

            PRE_PICK_TARGET,
            PICK_TARGET,

            PLAY_ACTIONS_DECIDE_ORDER,
            PRE_PLAY_ACTIONS,
            PLAY_ACTION_ATTACK,
            POST_PLAY_ACTIONS

        };

        enum BattleMenuState : int {
            IN_MAIN,
            IN_ENGAGE,
            IN_EVADE
        };
        
        namespace ActionType {
            enum ActionType : int {
                NONE,
                ATTACK,
                SAY,
                OFFER,
                BLOCK,
                DODGE,
                TANK,
                ESCAPE
            };  
            static String name(int type){
                switch(type){
                    case ATTACK: {
                        return "Attack";
                    } break;
                    case SAY: {
                        return "Say";
                    } break;  
                    case OFFER: {
                        return "Offer";
                    } break; 
                    case BLOCK: {
                        return "Block";
                    } break; 
                    case DODGE: {
                        return "Dodge";
                    } break; 
                    case TANK: {
                        return "Tank";
                    } break;      
                    case ESCAPE: {
                        return "Escape";
                    } break;                                                                                                                    
                    case NONE:
                    default: {
                        return "[nothing]";
                    } break;
                }
            }
        }

        struct DamageNumber : Object {
            int amount;
            nite::Batch batch;
            nite::Color color;
            UInt64 startTime;
            UInt64 lifetime;
            nite::Font font;
            nite::Vec2 offPosition;
            nite::Vec2 targetPosition;
            nite::Vec2 startPosition;
            UInt64 lastShakeTick;
            float angle;
            float targetAngle;
            DamageNumber();
            void step();
            void render();
            int animStep;
            void setPosition(const nite::Vec2 &p){
                this->position = p;
                this->lerpPos = p;
                this->targetPosition = p;
                this->startPosition = p;
            }            
        };

        struct BattleEntity {
            Shared<Entity> entity;
            int group;

			float battleAnimBlink;
			bool battlAnimBlinkFlip;
			UInt64 lastBattleAnimBlinkTick;

			UInt64 battleAnimTargetTime;
			int battleAnimStatus;
			float battleAnimTargetExp;
			UInt64 lastBattleAnimTick;
			UInt64 lastBattleAnimInnerTick;
			int battleAnimStep;
			nite::Vec2 battlAnimPosOff;
            
            nite::Vec2 position;
            BattleEntity();
			void setBattleAnim(int anim, UInt64 animTargetTime);
			bool isBattleAnim();
			void renderBattleAnim(float x, float y, bool blink);
        };


        struct ActionTurn {
            int type;
            Shared<BattleEntity> target;
            Shared<BattleEntity> owner;
            float fDmg;
        };

        struct Battle {
            nite::Font font;
            nite::Font subFont;
            nite::Texture selArrow;

            Vector<Shared<DamageNumber>> dmgNumbers;

            Shared<nite::BaseUIComponent> batWin;

            Vector<ActionTurn> decisions;
            int cdecision;

            Game::ActionTurn selAction;

            void addDmgNumber(const nite::Vec2 &stPos, int amnt);

            Shared<BattleEntity> getCurrentTurnSubject();
            Shared<BattleEntity> getCurrentSelTarget();
            void onSwitchSelTarget();

            int startTurn;
            int currentTurn;
            int state;
            int menuState;
            int selTarget;

            UInt64 onActionTimeout;
            UInt64 onActionTick;

            float selTargetOffset;
            UInt64 selTargetTick;
            bool selTargetFlip;

            UInt64 lastStChange;
            nite::Texture empty;
            nite::Vec2 actBarPos;
            
            Shared<Game::DialogHook> dialog;
            Vector<Shared<BattleEntity>> groupA;
            Vector<Shared<BattleEntity>> groupB;

            void setOptBoxVis(bool v);
            bool isOptBoxVis();

            void setDialogBoxVis(bool v);
            bool isDialogBoxVis();

            bool isShowing();
            void setState(int nstate);
            void nextTurn();

            void updOptBoxTitle(const String &str);

            Battle();
            void start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB);
            void end();
            void step();
            void render();


        };


    }


#endif