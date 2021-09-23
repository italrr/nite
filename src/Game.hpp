#ifndef GAME_CORE_HPP
    #define GAME_CORE_HPP

    #include "Dialog.hpp"
    #include "Battle.hpp"
    #include "Object.hpp"
    #include "World.hpp"
    #include "Storyline.hpp"
    #include "UI/ListMenu.hpp"
    
    namespace Game {

        struct GameCore {
            Shared<Game::UIListMenu> optionsMenu;
            Shared<Game::World> world;
            Shared<Game::DialogBox> dialog;
            Shared<Game::Battle> battle;
            Shared<Game::Entity> player;
            Shared<Game::Entity> mob; // temp
            Shared<Game::Entity> mob2; // temp
            Shared<Game::Story::InteractionDevice> interDevice;
            GameCore();
            void init();
            void step();
            void render();

        };

        Shared<Game::GameCore> getGameIns();

    }

#endif