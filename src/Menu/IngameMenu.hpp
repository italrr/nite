#ifndef GAME_INGAME_MENU_HPP
    #define GAME_INGAME_MENU_HPP

    #include "../Engine/UI/UI.hpp"
    #include "../Core/World.hpp"

    namespace Game {

        struct Client;
        struct InGameMenu {
            bool open;
            InGameMenu();
            Shared<nite::BaseUIComponent> main;
            Game::Client *client;
            void start(Game::Client *client);            
            void stop();
            void update();            
        };

    }

#endif