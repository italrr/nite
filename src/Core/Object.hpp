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
                Base = 0,
                Player,
                MobHumanoid
            };
            static String name(UInt16 sig){
                switch(sig){
                    case Player:
                        return "Player";
                    case MobHumanoid:
                        return "MobHumanoid";
                    default:
                        return "Undefined";
                }
            }
        }

        struct NetWorld;
        struct Server;
        struct Net;

        struct PredictFragment {
            nite::Vec2 pos;
            UInt32 order;
        };

        struct NetObject : nite::BaseObject {
            UInt16 id;
			int localId;	
			nite::Vec2 position;
			nite::Vec2 speed;
			nite::Vec2 size;
			float direction;
			float friction;
			float mass;
			virtual void destroy();
			float relativeTimescale;
			bool solid;
			bool unmovable;
			bool collided;

            UInt8 objType;
            UInt16 sigId;
            nite::Vec2 lerpPosition; // client side position interpolation
            nite::Vec2 lerpSpeed;
            nite::Vec2 nextPosition;
            nite::Vec2 clNextPosition;
            Game::NetWorld *container;
            Game::Server *sv;
            Game::Net *net;
            Int32 quadrant;
            nite::Vec2 lqPos; // last valid quadrant position
            void updateQuadrant();
            void clearQuadrant();

            UInt64 frictionRate;
            UInt64 lastMoveTime;            

            Vector<Game::PredictFragment> snapshots;
            
            NetObject();

            void snapPosition();
            void setPosition(const nite::Vec2 &p);
            void setPosition(float x, float y){
                setPosition(nite::Vec2(x, y));
                updateQuadrant();
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
			
			virtual bool move(const nite::Vec2 &dir);
			virtual bool push(const nite::Vec2 &dir);
			bool isCollidingWith(Game::NetObject *other);
			bool isCollidingWithSomething(Game::NetObject **who);
			bool isCollidingWithExcept(const Vector<Game::NetObject*> &ignores);
			float getDistance(Game::NetObject *other);

			virtual void onCollision(Game::NetObject *obj){

			}
        };

        Shared<Game::NetObject> createNetObject(UInt16 id, UInt16 sig, float x, float y);

    }


#endif