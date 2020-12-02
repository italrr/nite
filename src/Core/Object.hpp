#ifndef GAME_NETWORK_OBJECT_HPP
    #define GAME_NETWORK_OBJECT_HPP

    #include "../Engine/Object.hpp"
    #include "../Engine/Packets.hpp"


    namespace Game {

        namespace ObjectType {
            enum ObjectType : UInt8 {
                Base,
                Entity,
                Ghost,
                Prop,
                Wall,
                Projectile
            };
            static String name(UInt16 type){
                switch(type){
                    case Base:
                        return "Base";
                    case Entity:
                        return "Entity";
                    case Ghost:
                        return "Ghost";                        
                    case Prop:
                        return "Prop";                      
                    case Projectile:
                        return "Projectile";                        
                    default:
                        return "Undefined";
                }
            }            
        }

        namespace ObjectSig {
            enum ObjectSig :  UInt16 {
                Base = 0,
                Player,
                MobHumanoid,
                Projectile,
                Wall
            };
            static String name(UInt16 sig){
                switch(sig){
                    case Wall:
                        return "Wall";                      
                    case Player:
                        return "Player";
                    case MobHumanoid:
                        return "MobHumanoid";
                    case Projectile:
                        return "Projectile";                        
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

        namespace DeltaUpdateType {
            enum DeltaUpdateType : UInt8 {
                ANIMATION = 1,
                PHYSICS,
                SPECIAL
            };
            static const unsigned Total = 3;
            static const bool hasIssuedDeltaStateUpdate(UInt8 type, UInt8 state){  
                return state & (1 << type);                
            }
        }

        struct NetObject : nite::BaseObject {
            UInt8 deltaUpdates;
            void issueDeltaUpdate(UInt8 type);
            void clearDeltaUpdates();
            UInt16 id;
			int localId;	
			nite::Vec2 position;
			float speed;
            float direction;
			nite::Vec2 size;
			float friction;
			float mass;
			virtual void destroy();
            bool destroyed;
			float relativeTimescale;
			bool solid;
			bool unmovable;
			bool collided;
            Vector<NetObject*> locals;
            Vector<nite::Vec2> nextPosition;
            UInt8 objType;
            UInt16 sigId;
            nite::Vec2 lerpPosition; // client side position interpolation
            nite::Vec2 lerpSpeed;
            // nite::Vec2 nextPosition;
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
			
			virtual bool push(float dir, float force);
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