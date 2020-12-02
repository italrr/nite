#include "../Core/Network.hpp"
#include "Base.hpp"
#include "Projectile.hpp"



Game::Projectile::Projectile(){
    friction = 0.0f;
    mass = 1.0f;
    solid =  false;
    size.set(140, 68);
    type = ObjectType::Ghost;
    unmovable = false;
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
    // if(this->sv == NULL){
    //     this->texture.load(source.path, this->transparency);
    // }
    // nite::print(nite::toStr(dir)+" "+nite::toStr(spd));
    this->push(nite::toRadians(dir), spd);
}

void Game::Projectile::step(){
    if(!destroyed && toDestroy && nite::getTicks()-contactTime > 3000){
        nite::print("arrow destroyed");
        destroy();
    }
}

void Game::Projectile::onCollision(Game::NetObject *obj){
    if(obj->sigId == ObjectSig::Player){
        return;
    }
    if(obj->objType == ObjectSig::Wall && !toDestroy){
        this->speed = 0.0;
        toDestroy = true;
        contactTime = nite::getTicks();
    }
    
    // destroy();
}

void Game::Projectile::draw(){
	// float lrprate = (this->speed / 100.0f);
	// if(lrprate > 0.9f){
	// 	lrprate = 0.9f;
	// }
	// if(lrprate < 0.08f){
	// 	lrprate = 0.08f;
	// }
    // if(this->speed == 0.0f){
    //     lrprate = 0.45f;
    // }
    nite::Vec2 offset(frameSize.x * 0.5f, frameSize.y* 0.5f);
	lerpPosition.lerpDiscrete(position, 0.08f);
    int depth = -(position.y - size.y * 0.5f);
    nite::setDepth(depth);  
    texture.setRegion(inTexCoors, inTexSize);   
    texture.draw(offset.x + lerpPosition.x, offset.y + lerpPosition.y, frameSize.x, frameSize.y, origin.x, origin.y, dir);
}

void Game::Projectile::writeInitialState(nite::Packet &packet){
    packet.write(&dir, sizeof(dir));
    packet.write(&spd, sizeof(spd));
    packet.write(&owner, sizeof(owner));
}

void Game::Projectile::readInitialState(nite::Packet &packet){
    packet.read(&dir, sizeof(dir));
    packet.read(&spd, sizeof(spd));
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