#ifndef GAME_HUD_HPP
    #define GAME_HUD_HPP

    #include "../Engine/UI/UI.hpp"
    #include "../Network/World.hpp"

    namespace Game {

        struct HUD {
            Shared<nite::BaseUIComponent> main;
            UInt16 followId;
            Game::NetWorld *container;
            void start(Game::NetWorld *container);
            void setFollow(UInt16 followId);
            void stop();
            void update();

        };

    }

#endif 