#ifndef GAME_AI_HPP
  #define GAME_AI_HPP

  #include "../Engine/Tools/Tools.hpp"

  namespace Game {
    struct Entity;

    struct BaseBehavior {
      UInt64 behaviorTimeout;
      UInt64 lastBehaviorTimeout;
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

    struct OverworldBehavior : public BaseBehavior {

    };

    struct TestStareOB : public OverworldBehavior {
      bool excl;
      UInt64 lastWtf;
      void init();
      void think(Entity &subject);
    };
  }

#endif