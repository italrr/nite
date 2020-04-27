#ifndef GAME_INGAME_MENU_HPP
    #define GAME_INGAME_MENU_HPP

    #include "../Engine/UI/UI.hpp"
    #include "../Core/World.hpp"

    namespace Game {


        namespace MenuType {
            enum MenuType : UInt8 {
                Status = 0,
                Equip,
                Inventory,
                Skills,
                TBA,
                Map,
                Quest,
                Codex,
                Interact
            };
            static const UInt8 total = 9;
        }

        struct Client;
        struct InGameMenu {
            bool open;
            InGameMenu();
            Shared<nite::BaseUIComponent> main;
            Shared<nite::BaseUIComponent> subs[MenuType::total];
            void hideAllSubs();
            Game::Client *client;
            void start(Game::Client *client);            
            void stop();
            void update();            
        };

    }

#endif
