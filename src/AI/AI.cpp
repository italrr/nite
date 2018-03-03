#include "AI.hpp"
#include "../Entity.hpp"
#include "../Engine/Object.hpp"
#include "../Engine/World.hpp"
#include "../Game.hpp"

struct Exclamation : public nite::PhysicsObject {
  nite::Texture expr;
  UInt64 createdTimeout;
  float alpha;
  Exclamation(const nite::Vec2 &position){
    this->position.set(position);
  }
  void onCreate(){
    expr.load("data/sprite/exp_exclamation.png", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
    createdTimeout = nite::getTicks();
    alpha = 100.0f;
  }
  void step(){
    if(nite::getTicks()-createdTimeout > 1000){
      nite::lerp(alpha, 0.0f, 0.25f);
    }
    if(alpha <= 0)
      destroy();
  }
  void draw(){
    nite::setRenderTarget(nite::RenderTargetGame);
    nite::setColor(1.0f, 1.0f, 1.0f, alpha / 100.0f);
    nite::setDepth(nite::DepthTop);
    expr.draw(position.x, position.y, expr.getWidth() * 0.25f, expr.getHeight() * 0.25f, 0.5f, 0.5f, 0.0f);
  }
};

void Game::TestStareOB::init(){
  behaviorTimeout = 100;
  excl = false;
}

struct WTFExpression : public nite::PhysicsObject {
  nite::Texture face;
  nite::Vec2 facePosition;
  UInt64 timeout;
  float alpha;
  float dir;
  WTFExpression(const nite::Vec2 facePosition, int dir){
    this->facePosition.set(facePosition);
    face.load("data/sprite/wtf.png", nite::Color(0.0f, 0.0f, 1.0f, 1.0f));
    this->dir = dir == Game::EntityFacing::Right ? -1.0f : 1.0f;
  }

  void onCreate(){
    alpha = 100.0f;
    timeout = nite::getTicks();
  }

  void onDestroy(){

  }

  void update(){
    if(alpha == 0.0f){
      destroy();
      return;
    }
    if(nite::getTicks()-timeout > 800){
      nite::lerpDiscrete(alpha, 0.0f, 0.25f);
    }
  }

  void step(){

  }

  void draw(){
    nite::setRenderTarget(nite::RenderTargetGame);
    nite::setColor(1.0f, 1.0f, 1.0f, alpha / 100.0f);
    nite::setDepth(nite::DepthTop);
    nite::Vec2 cs(face.getWidth() * dir * 0.25f, face.getHeight() * 0.25f);
    face.draw(facePosition.x, facePosition.y - cs.y, cs.x, cs.y, 0.5f, 0.5f, 0.0f);
  }

};

void Game::TestStareOB::think(Entity &subject){
  auto &world = *subject.container;
  for (auto& it : world.objects){
    auto &target = *it.second.get();
    if(&target == &subject) continue;
    float x = nite::distance(subject.position, target.position);
    if(x > 450){
      if(excl) excl = false;
      continue;
    }
    subject.faceDirection = subject.position.x < target.position.x ? EntityFacing::Right : EntityFacing::Left;
    if(!excl){
      // world.add(Shared<nite::PhysicsObject>(new Exclamation(subject.position - nite::Vec2(0.0f, 92.0f))));
      excl = true;
      static auto inst = Game::getInstance();
    //   if(!inst->bs.isOnBattle()){
    //     Game::BattleGroup battle;
    //     auto cenemy = static_cast<Game::Entity*>(&subject);
    //     battle.add(static_cast<Game::Entity*>(&target), "player");
    //     battle.add(&subject, "enemy");
    //     inst->world.add(Shared<nite::PhysicsObject>(new WTFExpression(cenemy->position + cenemy->getHeadPosition(), cenemy->faceDirection)));
    //     inst->bs.battle(battle);
    //   }
      return;
    }
  }
}