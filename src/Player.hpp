#ifndef GAME_PLAYER_HPP
  #define GAME_PLAYER_HPP

  #include "Entity.hpp"

  namespace Game {
    struct Player : public Entity {
      void onCreate();
      void step();
    };
  }

#endif