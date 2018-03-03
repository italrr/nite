#ifndef GAME_MOB_HPP
  #define GAME_MOB_HPP

  #include <memory>
  #include "Entity.hpp"
  #include "AI/AI.hpp"

  namespace Game {
    struct BasicMob : public Entity {
      Vector<std::shared_ptr<Game::OverworldBehavior>> overworldAI;
      void onCreate();
      void add(std::shared_ptr<Game::OverworldBehavior> behavior);
      void step();
    };
  }

#endif