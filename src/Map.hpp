#ifndef GAME_MAP_HPP
  #define GAME_MAP_HPP

  #include "Engine/Tileset.hpp"
  #include "Engine/Object.hpp"

  namespace Game {

    namespace NavMapType {
      static const unsigned Empty = 0;
      static const unsigned SolidStatic = 1;
      static const unsigned SolidMovable = 2;
      static const unsigned Undefined = 999;
    }

    struct NavMap { // for ai
      nite::Vec2 size;
      nite::Vec2 mapBlockSize;
      nite::Vec2 nvBlockSize;
      nite::Vec2 nvSize;
      nite::Vec2 mapSize;
      int *grid; 
      void build(Vector<nite::PhysicsObject*> &locals, float nvmBs, float mBs, float width, float height);
      NavMap(Vector<nite::PhysicsObject*> &locals, float nvmBs, float mBs, float width, float height);
      NavMap();
    };

    struct Map {
      nite::Tileset tiles;
	    String path;
      Game::NavMap navMap;
	    nite::Color transparency;
	    Vector<nite::PhysicsObject*> locals;
      nite::Vec2 mapSize;
      nite::Vec2 tileSize;
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