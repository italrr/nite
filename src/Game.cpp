#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"
#include "Engine/Network.hpp"
#include "Engine/Shapes.hpp"

struct Object {

	nite::Vec2 position;
	nite::Vec2 vel;
	nite::Vec2 accel;
	nite::Vec2 size;
	nite::Vec2 lerpPos;
	float mass;
	float invMass;
	float friction;

	Object(){
		friction = 0.0f;
		setMass(0.0f);
		setPosition(nite::Vec2(0.0f));
	}

	virtual void create(){

	}

	virtual void step(){

	}

	virtual void render(){

	}

	virtual void onDestroy(){

	}

	void setMass(float mass){
		this->mass = mass;
		this->invMass = mass <= 0 ? 0.0f : 1.0f / mass;
	}

	void push(float force, float angle){
		this->accel.x += (nite::cos(angle) * (force / 100.0f)) / this->mass;
		this->accel.y += (nite::sin(angle) * (force / 100.0f)) / this->mass;
	}	

	void move(float force, float angle){
		nite::Vec2 nacc;
		nacc.x = (nite::cos(angle) * (force / 100.0f)) / this->mass;
		nacc.y = (nite::sin(angle) * (force / 100.0f)) / this->mass;


		float nmod = nite::sqrt(nacc.x * nacc.x + nacc.y * nacc.y);
		float mod = nite::sqrt(this->accel.x * this->accel.x + this->accel.y * this->accel.y);

		if(nmod >= mod){
			this->accel.x = nacc.x;
			this->accel.y = nacc.y;
		}else{
			this->accel.x += nacc.x;
			this->accel.y += nacc.y;
		}

	}

	void setPosition(const nite::Vec2 &p){
		this->position = p;
		this->lerpPos = p;
	}
};


struct World { 
	Vector<Shared<Object>> objects;
	UInt16 tickrate;
	UInt64 lastStep;

	void init(int width, int height){
		tickrate = 1000.0f / 60.0f;
		lastStep = nite::getTicks();
	}

	void step(){

		// run object step
		for(unsigned i = 0; i < objects.size(); ++i){
			auto &obj = objects[i];
			obj->step();
		}	

		// run physics
		if(nite::getTicks()-lastStep >= tickrate){
			float timescale = 1.0f;
			float dt = static_cast<float>(nite::getTicks()-lastStep) * timescale;
			lastStep = nite::getTicks();
			for(unsigned i = 0; i < objects.size(); ++i){
				auto &obj = objects[i];
				if(obj->accel.x == 0 && obj->accel.y == 0){
					continue;
				}
				obj->vel.x += obj->accel.x * dt;
				obj->vel.y += obj->accel.y * dt;
				obj->position.x += obj->vel.x * dt;
				obj->position.y += obj->vel.y * dt;
				// simulate friction
				float friction = 1.0f - (obj->friction * (dt/10.0f));
				// fix friction
				if(friction < 0.0f) friction = 0.01f;
				if(friction > 1.0f) friction = 1.0f;
				obj->vel.x *= friction;
				obj->vel.y *= friction;
				obj->accel.x *= friction;
				obj->accel.y *= friction;				
			}
		}
	}

	bool add(const Shared<Object> &obj){
		objects.push_back(obj);
		obj->create();
		return true;
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		for(unsigned i = 0; i < objects.size(); ++i){
			auto &obj = objects[i];
			obj->lerpPos.lerpDiscrete(obj->position, 0.1f);
			objects[i]->render();
		}
	}
};

struct Player : Object {

	void create(){
		setPosition(nite::Vec2(100));
		setMass(5.0f);
		this->friction = 0.10f;
	}
	void step(){
		if(nite::keyboardCheck(nite::keyW)){
			move(10.0f, nite::toRadians(90));
		}
		if(nite::keyboardCheck(nite::keyD)){
			move(10.0f, nite::toRadians(0));
		}
		if(nite::keyboardCheck(nite::keyS)){
			move(10.0f, nite::toRadians(270));
		}
		if(nite::keyboardCheck(nite::keyA)){
			move(10.0f, nite::toRadians(180));
		}						
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		nite::setDepth(-lerpPos.y);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		auto ref = nite::Draw::Rectangle(lerpPos.x, lerpPos.y, 64, 64, true, 0.5f, 0.5f, 0.0f);
	}
};

struct Obstacle : Object { 

	void create(){
		setPosition(nite::Vec2(500));
		setMass(5.0f);
		this->friction = 0.10f;
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		nite::setDepth(-lerpPos.y);
		nite::setColor(0.1f, 0.1f, 0.1f, 1.1f);
		auto ref = nite::Draw::Rectangle(lerpPos.x, lerpPos.y, 64, 64, true, 0.5f, 0.5f, 0.0f);	
	}
};

int main(int argc, char* argv[]){
	
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);

	nite::graphicsInit();
	nite::inputInit();

	World world;
	world.init(1000, 1000);
	world.add(std::make_shared<Player>(Player()));
	world.add(std::make_shared<Obstacle>(Obstacle()));
	
	while(nite::isRunning()){	
		nite::viewUpdate();
		nite::inputUpdate();

		world.step();
		world.render();

		nite::graphicsRender();
	}
	nite::graphicsEnd();


	return 0;
}