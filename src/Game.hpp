#ifndef GAME_CORE_HPP
    #define GAME_CORE_HPP

    #include "Dialog.hpp"
    #include "Battle.hpp"
    
    namespace Game {

        struct GameCore {

            Shared<DialogBox> dialog;
            Shared<Game::Battle> battle;
            GameCore();
            void init();
            void step();
            void render();

        };

        Shared<Game::GameCore> getGameIns();

    }

#endif