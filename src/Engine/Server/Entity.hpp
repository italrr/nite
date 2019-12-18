#ifndef ENTITY_HPP
  #define ENTITY_HPP
  #include "../Types.hpp"
    struct worldT;
    struct entityT {
      String name;
      int objectId;
      int Owner;
      unsigned walkTime;
      unsigned walkT;
      nite::Vec2 position;
      nite::Vec2 lerpPosition;
      worldT *World;
      virtual void create();
      virtual void destroy();
      virtual void update();
      float getWalkLerp();
      Vector<nite::Vec2> moveQueue;
      void move(int hSpaces, int vSpaces);
    };
#endif
