#ifndef GAME_HUD_HPP
    #define GAME_HUD_HPP

    #include "../Engine/UI/UI.hpp"
    #include "../Core/World.hpp"

    namespace Game {

        struct Client;
        struct HUD {
            Dict<UInt16, Shared<nite::BaseUIComponent>> lastEffectList;
            UInt64 lastCheck;
            HUD();
            Shared<nite::BaseUIComponent> main;
            UInt16 followId;
            Game::Client *client;
            void start(Game::Client *client);
            void setFollow(UInt16 followId);
            void resetValues();
            void stop();
            void update();
            void updateValues();

        };

    }

#endif 