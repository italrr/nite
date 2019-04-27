#ifndef GAME_MOB_HPP
  #define GAME_MOB_HPP

  #include <memory>
  #include "Entity.hpp"
  #include "AI/AI.hpp"

  namespace Game {
    struct BasicMob : public Entity {
      void onCreate();
      
      void step();

      // AI
      Vector<std::shared_ptr<Game::AI::BaseBehavior>> behaviors;
      void addBehavior(std::shared_ptr<Game::AI::BaseBehavior> behavior);
    };
  }

#endif