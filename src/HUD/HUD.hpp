#ifndef GAME_HUD_HPP
    #define GAME_HUD_HPP

    #include "../Engine/UI/UI.hpp"
    #include "../Core/World.hpp"

    namespace Game {

        struct Client;
        struct HUD {
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