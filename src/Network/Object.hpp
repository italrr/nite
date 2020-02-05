#ifndef GAME_NETWORK_OBJECT_HPP
    #define GAME_NETWORK_OBJECT_HPP

    #include "../Engine/Object.hpp"
    #include "../Engine/Packets.hpp"

    namespace Game {

        namespace ObjectType {
            static const UInt8 Base     = 0;
            static const UInt8 Entity   = 1;
            static const UInt8 Mob      = 2; // It's same as mob, but slightly modified
            static const UInt8 Prop     = 3;
        }

        struct NetWorld;
        struct NetObject : nite::PhysicsObject {
            UInt32 id;
            UInt8 type;
            Game::NetWorld *container;
            nite::Packet nextPacket;  
        };

        Shared<Game::NetObject> createNetObject(UInt8 type);

    }


#endif