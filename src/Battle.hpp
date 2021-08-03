#ifndef GAME_BATTLE_HPP
    #define GAME_BATTLE_HPP

    #include "Entity.hpp"
    #include "Dialog.hpp"

    #include "Dialog.hpp"

    namespace Game {


        enum BattleStatus : int {
            BATTLE_READY,
            TURN_START,
        };
        
        struct Battle {
            nite::Font font;
            nite::Font subFont;

            Shared<nite::BaseUIComponent> batWin;

            int status;
            nite::Texture empty;
            nite::Vec2 actBarPos;
            Shared<Game::DialogHook> dialog;
            Vector<Shared<Game::Entity>> groupA;
            Vector<Shared<Game::Entity>> groupB;

            Battle();
            void start(const Vector<Shared<Game::Entity>> &groupA, const Vector<Shared<Game::Entity>> &groupB);
            void end();
            void step();
            void render();


        };


    }


#endif