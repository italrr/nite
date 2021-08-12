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
            PICK_EVADE_OPT

        };
        
        enum ActionType : int {
            ATTACK,
        };

        struct ActionTurn {
            int type;
            int target;
            int owner;
        };

        struct Battle {
            nite::Font font;
            nite::Font subFont;

            Shared<nite::BaseUIComponent> batWin;

            Vector<ActionTurn> decisions;
            int cdecision;

            int startTurn;
            int currentTurn;
            int state;

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

            Battle();
            void start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB);
            void end();
            void step();
            void render();


        };


    }


#endif