#ifndef GAME_ENTITY_PROJECTILE_HPP
    #define GAME_ENTITY_PROJECTILE_HPP

    #include "../Core/Object.hpp"
    #include "../Engine/Texture.hpp"
    #include "../Engine/Indexer.hpp"
    
    #include "Inventory.hpp"

    namespace Game {

        namespace ProjectileType {
            enum ProjectType : UInt8 {
                Arrow = 0,
                Bullet
            };
        }

        struct Projectile : Game::NetObject {
            nite::Texture texture;
            nite::Vec2 inTexCoors;
            nite::Vec2 inTexSize;
            nite::Vec2 frameSize;
            nite::IndexedFile source;
            nite::Vec2 origin;
            nite::Color transparency;

            UInt64 contactTime;
            bool toDestroy;
            UInt8 type;
            UInt16 owner;

            nite::Vec2 dir;

            void setup(Game::AmmoItem *ammo);
            Projectile();
            void onCreate();
			void step();
            void draw();
            void onCollision(Game::NetObject *obj);
            void writeInitialState(nite::Packet &packet);
            void readInitialState(nite::Packet &packet);
        };

    }


#endif