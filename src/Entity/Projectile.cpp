#include "../Core/Network.hpp"
#include "Base.hpp"
#include "Projectile.hpp"



Game::Projectile::Projectile(){
    size.set(140, 68);
    body.rectangle(size);
    type = ProjectileType::Arrow;
    objType = ObjectType::Projectile;
    toDestroy = false;
}

void Game::Projectile::setup(Game::AmmoItem *ammo){
    this->frameSize = ammo->anim.frameSize;
    this->inTexCoors = ammo->anim.inTexCoors;
    this->inTexSize = ammo->anim.inTexSize;
    this->source = ammo->anim.source;
    this->origin = ammo->anim.origin;
    this->transparency = ammo->anim.transparency;
    if(this->sv == NULL){
        this->texture = ammo->anim.texture;
    }
}

void Game::Projectile::onCreate(){
    if(this->sv == NULL){
        this->texture.load(source.path, this->transparency);
    }
    // nite::print(nite::toStr(dir)+" "+nite::toStr(spd));
    // this->push(dir, spd);
}

void Game::Projectile::step(){
    if(!destroyed && toDestroy && nite::getTicks()-contactTime > 3000){
        nite::print("arrow destroyed");
        destroy();
    }
    // if(!this->move(dir.x, dir.y) && !toDestroy){
    //     toDestroy = true;
    //     contactTime = nite::getTicks();
    // }
}

void Game::Projectile::onCollision(Game::NetObject *obj){
    if(obj->sigId == ObjectSig::Player){
        return;
    }
    // if(obj->objType == ObjectSig::Wall && !toDestroy && nite::getTicks()-contactTime > 32){
    //     // this->speed = 0.0;
    //     // this->direction += nite::toRadians(180);
    //     issueDeltaUpdate(DeltaUpdateType::PHYSICS);
    //     // toDestroy = true;
    //     contactTime = nite::getTicks();
    // }
    
    // destroy();
}

void Game::Projectile::draw(){
    nite::Vec2 offset(frameSize.x * 0.5f, frameSize.y* 0.5f);
    int depth = -(rPosition.y - size.y * 0.5f);
    nite::setDepth(depth);  
    texture.setRegion(inTexCoors, inTexSize);   
    texture.draw(rPosition.x, rPosition.y, frameSize.x, frameSize.y, 0.0f, 0.0f, 0.0f);
}

void Game::Projectile::writeInitialState(nite::Packet &packet){
    packet.write(dir);
    packet.write(&owner, sizeof(owner));
}

void Game::Projectile::readInitialState(nite::Packet &packet){
    packet.read(dir);
    packet.read(&owner, sizeof(owner));
    auto obj = this->net->world.get(owner);
    if(obj == NULL){
        nite::print("Projectile::readInitialState: failed to find obj id '"+nite::toStr(owner)+"' owner of this projectile");
        return;
    }
    if(obj->objType != ObjectType::Entity){
        nite::print("Projectile::readInitialState: obj id '"+nite::toStr(owner)+"' is not an entity");
        return;
    }
    auto ent = static_cast<Game::EntityBase*>(obj);
    if(ent->invStat.activeAmmo == NULL){
        nite::print("Projectile::readInitialState: ent id '"+nite::toStr(owner)+"' is not wearing an ammo right now (?)");
        return;
    }
    this->setup(ent->invStat.activeAmmo);
}