#include "Bullet.hpp"
#include "Engine/Texture.hpp"

struct ShootAnim : nite::PhysicsObject {
    nite::Animation anim;
    unsigned frame;
    UInt64 lifetime;
    UInt64 createdAt;
    void onCreate(){
        anim.load("data/sprite/shoot_anim.png", nite::Color(255, 0, 0));
        frame = anim.add(0, 0, 32, 32, 2, 0.08f, false, false);
        createdAt = nite::getTicks();
        lifetime = 250; // msecs
        solid = false;
        unmovable = false;
        friction = 0.0f;
        mass = 0.0f;                    
    }

    void onDestroy(){

    }

    void step(){
        if(nite::getTicks()-createdAt > nite::timescaled(lifetime)){
            destroy();
        }
    }
    
    void draw(){
	    nite::setRenderTarget(nite::RenderTargetGame);
	    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	    nite::setDepth(nite::DepthMiddle);        
        anim.draw(frame, position);
    }                
};

void Game::BaseBullet::setup(Game::Entity *owner, float mod, float angle, int weapDmg, int bulletDmg, UInt64 lifetime){
    this->owner = owner;
    this->mod = mod;
    this->angle = angle;
    this->weapDmg = weapDmg;
    this->bulletDmg = bulletDmg;
    this->lifetime = lifetime;
    this->totalDmg = weapDmg + bulletDmg;
}

void Game::BaseBullet::onCreate(){
    unmovable = false;
    solid = false;
    friction = 0.01f; 
    mass = 0.04f;
    size.set(8.0f, 8.0f);
    move(angle, mod);
    createdAt = nite::getTicks();
}

void Game::BaseBullet::onDestroy(){

}

void Game::BaseBullet::step(){
    if(nite::getTicks() - createdAt > nite::timescaled(lifetime)){
        destroy();
    }
}

void Game::BaseBullet::draw(){
    static nite::Texture tex("data/sprite/bullet.png", nite::Color(1.0f, 1.0f, 1.0f));
	nite::setRenderTarget(nite::RenderTargetGame);
	nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
	nite::setDepth(nite::DepthMiddle);
    tex.draw(position.x, position.y, tex.getWidth(), tex.getHeight(), 0.5f, 0.5f, 0.0f);
}