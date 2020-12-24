#ifndef GAME_NETWORK_OBJECT_HPP
    #define GAME_NETWORK_OBJECT_HPP

    #include "../Engine/Object.hpp"
    #include "../Engine/Packets.hpp"
    #include "../Entity/Anim.hpp"

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

        struct World;
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

        struct ObjectState {
            UInt32 delta;
            UInt16 objId;
            UInt8 states;
            UInt8 faceDir;
            UInt8 animSt[AnimPart::total];
            UInt8 animNum[AnimPart::total];
            UInt16 animExtime[AnimPart::total];
            float xLookingAt;
            float yLookingAt;
            float direction;
            float orientation;
            float speed; 
            float x;
            float y;
            ObjectState(){
                delta = 0;
            }
        };

        struct NetObject : nite::BaseObject {
            UInt8 deltaUpdates;
            void issueDeltaUpdate(UInt8 type);
            void clearDeltaUpdates();
            nite::Polygon body;
            UInt16 id;
			nite::Vec2 position;
            nite::Vec2 rPosition;
			nite::Vec2 size;
			virtual void destroy();
            bool destroyed;
            float speed; // cell per tick
			float relativeTimescale;
			bool collided;
            UInt8 objType;
            UInt16 sigId;
            void runState();
            Game::ObjectState prevState;
            Game::ObjectState nextState;
            Game::ObjectState currentState;
            Game::World *container;
            Game::Server *sv;
            Game::Net *net;

            void setPosition(const nite::Vec2 &p);

            NetObject();

            virtual void writeInitialState(nite::Packet &packet){}
            virtual void readInitialState(nite::Packet &packet){}
			
			virtual bool move(int x, int y);
			float getDistance(Game::NetObject *other);

			virtual void onCollision(Game::NetObject *obj){}
        };

        Shared<Game::NetObject> createNetObject(UInt16 sig, float x, float y);

    }


#endif