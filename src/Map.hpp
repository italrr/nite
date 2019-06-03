#ifndef GAME_MAP_HPP
  #define GAME_MAP_HPP

  #include "Engine/Tileset.hpp"
  #include "Engine/Object.hpp"

  namespace Game {

    namespace NavMapType {
      static const unsigned Empty = 0;
      static const unsigned SolidStatic = 1;
      static const unsigned SolidMovable = 2;
      static const unsigned undefined = 999;
    }

    struct NavMap { // for ai
      nite::Vec2 size;
      size_t totalSize;
      nite::Vec2 blockSize;
      int *grid; 
      void build(Vector<nite::PhysicsObject*> &locals);
    };

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