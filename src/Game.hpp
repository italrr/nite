#ifndef GAME_H
  #define GAME_H

  #include <memory>
  #include "Engine/Graphics.hpp"
  #include "Engine/Input.hpp"
  #include "Engine/View.hpp"
  #include "Engine/Object.hpp"
  #include "Engine/World.hpp"
  #include "Engine/Texture.hpp"
  #include "Engine/Tools/Tools.hpp"
  #include "Engine/Shader.hpp"
  #include "Engine/Console.hpp"
  #include "Engine/Font.hpp"
  #include "Engine/UI/UI.hpp"
  #include "Entity.hpp"
  #include "Player.hpp"
  #include "Mob.hpp"
  #include "Map.hpp"
  #include "Inventory.hpp"

  namespace Game {

	struct Camera {
	  int followId;
	  void update();
	  void update(nite::Vec2 &v, float mu);
	  void update(nite::Vec2 &v);
	  void follow(int id);
	  Camera();
	};

	struct GameMaster {
	  Game::Camera camera;
	  Game::Map map;
	  nite::World world;
	  nite::UIMaster ui;
	  Game::Entity *player;
	  bool isRunning;
	  bool renderEntities;
	  void start();
	  void update();
	  void render();
	  void end();
	  void onEnd();
	
	};

	Game::GameMaster *getInstance();

  }  

#endif
