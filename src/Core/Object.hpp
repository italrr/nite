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
        struct Server;
        struct Net;
        struct NetObject : nite::PhysicsObject {
            UInt8 objType;
            UInt16 sigId;
            Game::NetWorld *container;
            Game::Server *sv;
            Game::Net *net;

            NetObject(){
                container = NULL;
                sv = NULL;
                net = NULL;
                objType = Game::ObjectType::Base;
            }


            // the idea behind these is that we'll be updating
            // specific things from entities/objects but they'll read and prove it themselves
            // we should every entity with the changeForSync flag to be true

            bool readyForSync;
            // we'll be fetching the initial values to be sync'd
            virtual void writeInitialState(nite::Packet &packet){

            }
            virtual void readInitialState(nite::Packet &packet){

            }
        };

        Shared<Game::NetObject> createNetObject(UInt16 id, UInt16 sig, float x, float y);

    }


#endif