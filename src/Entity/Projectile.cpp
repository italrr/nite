#include "Projectile.hpp"


Game::Projectile::Projectile(){
    friction = 0.0f;
    mass = 1.0f;
    solid =  true;
    size.set(140, 68);
    type = ObjectType::Projectile;
    unmovable = false;
}

void Game::Projectile::load(const String &path){

}

void Game::Projectile::onCreate(){
    if(this->sv == NULL){
        texture.load("data/texture/ammo/normal_arrow.png", nite::Color("#ff0000"));
    }
    // nite::print(nite::toStr(dir)+" "+nite::toStr(spd));
    this->push(nite::toRadians(dir), spd);
}

void Game::Projectile::step(){
    if(this->sv != NULL){
        nite::print(position);
    }
}

void Game::Projectile::onCollision(Game::NetObject *obj){
    nite::print("arrow destroyed");
    destroy();
}

void Game::Projectile::draw(){
    lerpPosition.lerpDiscrete(position, 0.21f);
    texture.setRegion(nite::Vec2(0.0f), nite::Vec2(140, 68)); 
    texture.draw(lerpPosition.x + 140 * 0.5f, lerpPosition.y + 68 * 0.5f, 140, 68, 0.5f, 0.5f, dir);
}

void Game::Projectile::writeInitialState(nite::Packet &packet){
    packet.write(&dir, sizeof(dir));
    packet.write(&spd, sizeof(spd));
}

void Game::Projectile::readInitialState(nite::Packet &packet){
    packet.read(&dir, sizeof(dir));
    packet.read(&spd, sizeof(spd));
}