#ifndef GAME_HUD_HPP
    #define GAME_HUD_HPP

    #include "../Engine/UI/UI.hpp"
    #include "../Core/World.hpp"

    namespace Game {

        struct Client;
        
        struct ActionableHUDObject {
            Shared<nite::BaseUIComponent> cmp;
            float recharge;
            float alpha;
            ActionableHUDObject(){
                alpha = 100.0f;
                recharge = 100.0f;
            }
        };

        struct HUD {
            Dict<UInt16, Shared<nite::BaseUIComponent>> lastEffectList;
            UInt64 lastCheck;
            HUD();
            Vector<Game::ActionableHUDObject> actionables;
            Shared<nite::BaseUIComponent> main;
            UInt16 followId;
            nite::Shader actShader;
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