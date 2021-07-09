#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"
#include "Engine/Network.hpp"
#include "Engine/Shapes.hpp"

#include "World.hpp"


struct Player : Game::Object {

	void create(){
		setPosition(nite::Vec2(100));
		setMass(5.0f);
		this->friction = 1.0f;
	}
	
	void step(){
		if(nite::keyboardCheck(nite::keyW)){
			move(0.0f, -2.0f);
		}
		if(nite::keyboardCheck(nite::keyD)){
			move(2.0f, 0.0f);
		}
		if(nite::keyboardCheck(nite::keyS)){
			move(0.0f, 2.0f);
		}
		if(nite::keyboardCheck(nite::keyA)){
			move(-2.0f, 0.0f);
		}			
		if(nite::keyboardPress(nite::keyR)){
			this->orientation += 0.1f;
		}
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		nite::setDepth(-lerpPos.y);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		auto ref = nite::Draw::Rectangle(lerpPos.x, lerpPos.y, size.x, size.y, true, 0.5f, 0.5f, nite::toDegrees(this->orientation));
	}
};

struct Obstacle : Game::Object { 

	void create(){
		setPosition(nite::Vec2(500));
		setMass(5.0f);
		this->friction = 1.0f;
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		nite::setDepth(-lerpPos.y);
		nite::setColor(0.1f, 0.1f, 0.1f, 1.1f);
		auto ref = nite::Draw::Rectangle(lerpPos.x, lerpPos.y, size.x, size.y, true, 0.5f, 0.5f, nite::toDegrees(this->orientation));	
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

	Game::World world;
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