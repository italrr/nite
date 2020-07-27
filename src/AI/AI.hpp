#ifndef GAME_AI_HPP
#define GAME_AI_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Map.hpp"

    namespace Game {
        
        struct EntityBase;
        struct AIDriver;
        namespace AI {

            enum BehaviorType : UInt8 {
                Idle = 0,
                Offensive,
                Defensive,
                Wanderer,
                Gatherer
            };

            enum MemoryType : int {
                Hint, 
                Intent, 
                
            };

            enum MemoryTag : int {
                Seen,
                Attacked,
                Defended,
                Traded,
                Talked
            };

            struct Memory {
                AI::MemoryTag tag;
                int artifactId; // whether entityId, itemId, skillId
                AI::MemoryType type;
                UInt64 creation;
                UInt64 expiration;
                nite::Vec2 position;
                UInt16 author; // entityId
            };

            struct Notion {
                String name;
                String desc;
                int value;
                int ref;
                nite::Vec2 loc;
                AI::BehaviorType type; // behaviortype
                UInt16 author; // entityId
            };

            
            struct Concious {
                Vector<AI::Memory> memory;
                Vector<AI::Notion> notions;
                void subscribe(AIDriver *driver);
            };

            struct Hivemind {

            };
            
            struct BaseBehavior {
                UInt64 tickrate;
                UInt64 lastTick;
                /*
                    Priority will decide which behavior is executed first, and  
                    how strong is its authority agaisnt other baheviors.

                    A beahvior with higher priority than others will have the power
                    to alter, and even completely drop execution in future frames of
                    lesser behaviors.

                    Priority is defined in descending order. Meaning 0 will always be
                    the behavior with the highest authority.
                */
                int priority;
                /*
                    Main purpose of type is to be able to  quickly tell behaviors
                    apart by their nature.
                */
                unsigned type;
                String name;
                Vector<String> tags;
                BaseBehavior(){
                    lastTick = 32;
                }
                virtual bool isReady(){
                    if(nite::getTicks()-lastTick > tickrate){
                        tickrate = nite::getTicks();
                        return true;
                    }
                    return false;
                }
                virtual void init(){

                }
                virtual void think(EntityBase *subject){

                }
            };

            struct WanderBehavior : public BaseBehavior {
                void init();
                void think(Game::EntityBase *subject);
            };

            struct FollowBehavior : public BaseBehavior {
                void init();
                void think(Game::EntityBase *subject);
            };            

            struct DumbassBehavior : public BaseBehavior {
                int rand;
                UInt64 lastRand;
                UInt64 nextRand;
                void init();
                void think(Game::EntityBase *subject);
            };

        }
    }

#endif