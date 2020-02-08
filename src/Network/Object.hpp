#ifndef GAME_NETWORK_OBJECT_HPP
    #define GAME_NETWORK_OBJECT_HPP

    #include "../Engine/Object.hpp"
    #include "../Engine/Packets.hpp"

    namespace Game {

        namespace ObjectType {
            enum ObjectType : UInt8 {
                Base,
                Entity,
                Prop
            };
            static String name(UInt16 type){
                switch(type){
                    case Base:
                        return "Base";
                    case Entity:
                        return "Entity";
                    case Prop:
                        return "Prop";
                    default:
                        return "Undefined";
                }
            }            
        }

        namespace ObjectSig {
            enum ObjectSig :  UInt16 {
                Player = 0
            };
            static String name(UInt16 sig){
                switch(sig){
                    case Player:
                        return "Player";
                    default:
                        return "Undefined";
                }
            }
        }

        struct NetWorld;
        struct NetObject : nite::PhysicsObject {
            UInt16 id;
            UInt8 objType;
            UInt16 sigId;
            Game::NetWorld *container;
            nite::Packet nextPacket;  
        };

        Shared<Game::NetObject> createNetObject(UInt16 id, UInt16 sig, float x, float y);

    }


#endif