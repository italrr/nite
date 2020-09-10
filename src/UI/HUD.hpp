#ifndef GAME_HUD_HPP
    #define GAME_HUD_HPP

    #include "../Engine/Shader.hpp"
    #include "../Engine/Font.hpp"
    #include "../Engine/Texture.hpp"

    #include "../Core/World.hpp"


    #include "Icon.hpp"

    namespace Game {

        struct Client;
        
        struct ActionableHUDObject {
            UInt8 type;
            UInt32 id; // either slotId or skill id            
            Game::Icon icon;
            float recharge;
            float alpha;
            nite::Vec2 position;
            ActionableHUDObject();
        };

        struct EffectHUDObject {
            UInt16 insId;
            UInt16 efId;
            Int64 timeleft; // < 0 -> infinite
            nite::Color color;
            nite::Vec2 position;
            Game::Icon icon;
            nite::Vec2 size;
            bool destroy;
            EffectHUDObject(){
                destroy = false;
                size.set(0.0f);
            }
        };

        struct HUD {
            nite::Vec2 efSize;
            nite::Vec2 actSize;
            nite::Vec2 hlthSize;

            nite::Batch batch;
            nite::Texture empty;
            nite::Font healthFont;
            nite::Font healthSmallFont;

            float pHealth;
            float pHealthTarget;
            float pHealthMax;
            
            float pMana;
            float pManaTarget;
            float pManaMax;
            
            float pStamina;
            float pStaminaTarget;
            float pStaminaMax;

            nite::Vec2 healthPos;
            nite::Vec2 realPos;

            Dict<UInt16, EffectHUDObject> effects;
            Vector<ActionableHUDObject> actionables;
            UInt64 lastCheck;
            HUD();
            //Shared<nite::BaseUIComponent> main;
            UInt16 followId;
            nite::Shader actShader;
            nite::Shader efShader;
            Game::Client *client;
            void start(Game::Client *client);
            void follow(UInt16 followId);
            void stop();
            void update();
            void draw();
            void rerender();

        };

    }

#endif 