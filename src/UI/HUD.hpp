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

        struct EffectHUDObject {
            UInt16 insId;
            UInt16 efId;
            int cmpId;
            float runtime;
            float alpha;
            bool done;
            nite::Color color;
            Shared<nite::BaseUIComponent> cmp;
            EffectHUDObject(){
                runtime = 100.0f;
                alpha = 0.0f;
                done = false;
            }
        };

        struct HUD {
            Vector<EffectHUDObject> effects;
            UInt64 lastCheck;
            HUD();
            Vector<Game::ActionableHUDObject> actionables;
            Shared<nite::BaseUIComponent> main;
            UInt16 followId;
            nite::Shader actShader;
            nite::Shader efShader;
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