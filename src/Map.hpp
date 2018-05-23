#ifndef GAME_MAP_HPP
  #define GAME_MAP_HPP

  #include "Engine/Tileset.hpp"
  #include "Engine/Object.hpp"

  namespace Game {
    struct Map {
      nite::Tileset tiles;
	  String path;
	  nite::Color transparency;
	  Vector<nite::PhysicsObject*> locals;
	  nite::Vec2 playerSpawn;
      void load(const String &path, const nite::Color &transparency);
      void load(const String &path);
      void update();
      void render();
	  void reload();
      void clear();
    };
  }

#endif