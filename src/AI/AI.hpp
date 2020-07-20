#ifndef GAME_AI_HPP
#define GAME_AI_HPP

    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Map.hpp"

    namespace Game {
        
        struct EntityBase;
        namespace AI {

            enum Type : UInt8 {
                Idle = 0,
                Offensive,
                Defensive,
                Wanderer,
                Gatherer
            };

            enum EventType : int {

            };

            struct MemoryEvent {
                String tag;
                int id; // whether entityId, itemId, skillId
                int type;
                UInt64 time;
                nite::Vec2 pos;
            };

            struct SharedMemory {
                
            };
            
            
            struct BaseBehavior {
                UInt64 behaviorTimeout;
                UInt64 lastBehaviorTimeout;
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
                    behaviorTimeout = 32;
                }
                virtual bool isReady(){
                    if(nite::getTicks()-lastBehaviorTimeout > behaviorTimeout){
                        lastBehaviorTimeout = nite::getTicks();
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