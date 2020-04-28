#ifndef GAME_INGAME_MENU_HPP
    #define GAME_INGAME_MENU_HPP

    #include "../Engine/UI/UI.hpp"
    #include "../Core/World.hpp"
    #include "../Entity/Inventory.hpp"

    namespace Game {


        namespace MenuType {
            enum MenuType : UInt8 {
                Status = 0,
                Equip,
                Inventory,
                Skills,
                Craft,
                Map,
                Quest,
                Codex,
                Interact
            };
            static const UInt8 total = 9;
        }

        struct InGameMenuInvCached {
            Shared<Game::ItemBase> item;
            Shared<nite::BaseUIComponent> cmp;
        };

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
            // Inventory
            Dict<UInt16, InGameMenuInvCached> invCached; // key is slot id 
            void invUpdateCached();      
        };

    }

#endif
