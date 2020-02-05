#include "AI.hpp"
#include "../Entity.hpp"
#include "../Engine/Object.hpp"
#include "../Engine/World.hpp"
#include "../Game.hpp"
#include "../Player.hpp"

void Game::AI::StareBehavior::init(){
  behaviorTimeout = 100;
  priority = 10;
  type = Game::AI::Type::Sentinel;
  name = "Stare";
}

void Game::AI::StareBehavior::think(Entity &subject){
  auto &world = *subject.container;
  // for (auto& it : world.objects){
  //   auto &target = *it.second.get();
  //   if(&target == &subject) continue;
  //   float x = nite::distance(subject.position, target.position);
  //   if(x > 450){
  //     continue;
  //   }
  //   subject.faceDirection = subject.position.x < target.position.x ? EntityFacing::Right : EntityFacing::Left;
  // }
}


void Game::AI::DumbFollowBehavior::init(){
  behaviorTimeout = 8;
  priority = 11;
  type = Game::AI::Type::Sentinel;
  name = "DumbFollow";
}

void Game::AI::DumbFollowBehavior::think(Entity &subject){
	static auto game = Game::getGameCoreInstance();

  // find player
  // auto &player = *static_cast<Game::Player*>(game->player);
  // auto angle = nite::arctan(player.position.y - subject.position.y, player.position.x - subject.position.x);
  // subject.entityMove(angle, subject.walkPushRate);

}