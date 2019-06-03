#ifndef GAME_AI_HPP
  #define GAME_AI_HPP

  #include "../Engine/Tools/Tools.hpp"

  namespace Game {
    struct Entity;

    namespace AI {

      namespace Type {
        static const unsigned Idle = 0;
        static const unsigned Offensive = 1;
        static const unsigned Defensive = 2;
        static const unsigned Sentinel = 3;
        static const unsigned Gather = 4;
			  static const unsigned Undefined = 999;        
      }

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
        virtual bool isReady(){
          if(nite::getTicks()-lastBehaviorTimeout > behaviorTimeout){
            lastBehaviorTimeout = nite::getTicks();
            return true;
          }
          return false;
        }
        virtual void init(){

        }
        virtual void think(Entity &subject){

        }
      };


      // Basic behaviors

      struct StareBehavior : public BaseBehavior {
        void init();
        void think(Entity &subject);
      };

      struct DumbFollowBehavior : public BaseBehavior {
        void init();
        void think(Entity &subject);
      };


    }
  }

#endif