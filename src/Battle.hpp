#ifndef GAME_BATTLE_HPP
    #define GAME_BATTLE_HPP

    #include "Entity.hpp"
    #include "Dialog.hpp"

    #include "Dialog.hpp"

    #include "Vfx.hpp"

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
            POST_PLAY_ACTIONS,

            ENTITY_DEATH,
            DEATH_EXIT,

            GAME_OVER,
            BATTLE_END

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

        enum DamageNumberPattern : int {
            FALL_LEFT,
            DROP_RIGHT
        };

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
            int pattern;
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
            nite::Font font;
            nite::Font subFont;            
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
            
            nite::Vec2 posOffset;
            nite::Vec2 posLerp;

            bool died;
            bool isOut();
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
            nite::Texture selArrowBlack;
            nite::Texture selArrowWhite;
            nite::Texture empty;

            Game::VfxDevice vfxDev;

            Vector<Shared<DamageNumber>> dmgNumbers;

            Shared<nite::BaseUIComponent> batWin;

            Vector<ActionTurn> decisions;
            int cdecision;

            Game::ActionTurn selAction;

            // Shared<BattleEntity> whoDied;

            Vector<Shared<BattleEntity>> whoDied;

            void addDmgNumber(const nite::Vec2 &stPos, int amnt, int pattern = DamageNumberPattern::FALL_LEFT);

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

            float diagArrowOffset;
            UInt64 diagArrowTick;
            bool diagArrowFlip;            

            UInt64 lastStChange;
            nite::Vec2 actBarPos;
            nite::Vec2 batWinPos;
            nite::Vec2 shakeOffPos;
            nite::Vec2 playerStatPos;
            
            Shared<Game::DialogHook> dialog;
            Vector<Shared<BattleEntity>> groupA;
            Vector<Shared<BattleEntity>> groupB;

            bool shakeDamageEff;
            UInt64 shakeDmgEffTick;
            UInt64 shakeDmgEffAppTick;


            std::function<void(int winGroup)> onBattleEnd;

            void shakeEff();

            void setOptBoxVis(bool v);
            bool isOptBoxVis();

            void setDialogBoxVis(bool v);
            bool isDialogBoxVis();


            void setWinVis(bool v);
            bool isWinVis();

            bool isShowing();
            void setState(int nstate);
            void nextTurn();

            void end(int winGroup);

            void updOptBoxTitle(const String &str);

            Battle();
            void reset();
            void start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB);
            void step();
            void render();


        };


    }


#endif