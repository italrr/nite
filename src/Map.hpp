#ifndef GAME_MAP_HPP
  #define GAME_MAP_HPP


  #include "Engine/Tileset.hpp"

  namespace Game {
    struct Map {
      nite::Tileset tiles;
      void load(const String &path, const nite::Color &transparency);
      void load(const String &path);
      void update();
      void render();
      void clear();
    };
  }

#endif