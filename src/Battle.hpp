#ifndef GAME_BATTLE_HPP
    #define GAME_BATTLE_HPP

    #include "Entity.hpp"
    #include "Dialog.hpp"

    #include "Dialog.hpp"

    namespace Game {


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
            PLAY_ACTIONS

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

        struct ActionTurn {
            int type;
            Shared<Entity> target;
            Shared<Entity> owner;
        };

        struct Battle {
            nite::Font font;
            nite::Font subFont;
            nite::Texture selArrow;

            Shared<nite::BaseUIComponent> batWin;

            Vector<ActionTurn> decisions;
            int cdecision;

            Game::ActionTurn selAction;

            Shared<Entity> getCurrentTurnSubject();
            Shared<Entity> getCurrentSelTarget();
            void onSwitchSelTarget();

            int startTurn;
            int currentTurn;
            int state;
            int menuState;
            int selTarget;

            float selTargetOffset;
            UInt64 selTargetTick;
            bool selTargetFlip;

            UInt64 lastStChange;
            nite::Texture empty;
            nite::Vec2 actBarPos;
            
            Shared<Game::DialogHook> dialog;
            Vector<Shared<Game::Entity>> groupA;
            Vector<Shared<Game::Entity>> groupB;

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