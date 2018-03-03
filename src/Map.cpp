#include "Map.hpp"


void Game::Map::load(const String &path, const nite::Color &transparency){
  tiles.load(path);
  tiles.loadSources(transparency);
}


void Game::Map::load(const String &path){
  load(path, nite::Color(-1.0f, -1.0f, -1.0f, -1.0f));
}

void Game::Map::update(){

}

void Game::Map::render(){
  nite::setDepth(0.0);
  nite::setRenderTarget(nite::RenderTargetGame);
  nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
  tiles.draw(0, 0);
}

void Game::Map::clear(){

}