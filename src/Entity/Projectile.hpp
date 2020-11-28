#ifndef GAME_ENTITY_PROJECTILE_HPP
    #define GAME_ENTITY_PROJECTILE_HPP

    #include "../Core/Object.hpp"
    #include "../Engine/Texture.hpp"
    #include "../Engine/Indexer.hpp"

    namespace Game {

        namespace ProjectileType {
            enum ProjectType : UInt8 {
                Arrow = 0,
                Bullet
            };
        }

        struct Projectile : Game::NetObject {
            nite::Texture texture;
            nite::Vec2 inCoorsTex;
            nite::Vec2 frameSize;
            nite::IndexedFile source;
            UInt8 type;
            float dir;
            float spd;
            void load(const String &path);
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