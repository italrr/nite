#ifndef GAME_CORE_HPP
    #define GAME_CORE_HPP

    #include "Dialog.hpp"
    #include "Battle.hpp"
    #include "Object.hpp"
    #include "World.hpp"
    
    namespace Game {

        struct GameCore {
            Shared<Game::World> world;
            Shared<DialogBox> dialog;
            Shared<Game::Battle> battle;
            Shared<Game::Entity> player;
            Shared<Game::Entity> mob; // temp
            GameCore();
            void init();
            void step();
            void render();

        };

        Shared<Game::GameCore> getGameIns();

    }

#endif