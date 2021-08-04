#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"
#include "Engine/Network.hpp"
#include "Engine/Shapes.hpp"

#include "World.hpp"
#include "Dialog.hpp"
#include "Battle.hpp"


struct Player : Game::Entity {

	void create(){
		setPosition(nite::Vec2(200, 500));
		setMass(1.0f);
		this->friction = 4.0f;
		this->setupStat(1);
	}
	
	void step(){
		if(nite::keyboardCheck(nite::keyW)){
			move(0.0f, -1.0f);
		}
		if(nite::keyboardCheck(nite::keyD)){
			move(1.0f, 0.0f);
		}
		if(nite::keyboardCheck(nite::keyS)){
			move(0.0f, 1.0f);
		}
		if(nite::keyboardCheck(nite::keyA)){
			move(-1.0f, 0.0f);
		}			
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		nite::setDepth(-lerpPos.y);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		auto ref = nite::Draw::Rectangle(lerpPos.x, lerpPos.y, size.x, size.y, true, 0.5f, 0.5f, 0.0f);
	}
};


struct Mob : Game::Entity {

	void create(){
		setPosition(nite::Vec2(500, 500));
		setMass(1.0f);
		this->friction = 4.0f;
		this->setupStat(1);
	}
	
	void step(){
		
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		nite::setDepth(-lerpPos.y);
		nite::setColor(1.0f, 0.0f, 1.0f, 1.0f);
		auto ref = nite::Draw::Rectangle(lerpPos.x, lerpPos.y, size.x, size.y, true, 0.5f, 0.5f, 0.0f);
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
		auto ref = nite::Draw::Rectangle(lerpPos.x, lerpPos.y, size.x, size.y, true, 0.5f, 0.5f, 0.0f);	
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

	auto player = std::make_shared<Player>(Player());
	auto mob = std::make_shared<Mob>(Mob());

	Game::World world;
	world.init(1000, 1000);
	world.add(player);
	world.add(mob);
	
	auto dialog = Shared<Game::DialogBox>(new Game::DialogBox());
	auto battle = Shared<Game::Battle>(new Game::Battle());
	// battle->start({player}, {mob});
	// battle->setDialog(dialog);

	dialog->add("betsy", "this is a line of text", nite::Color("#d20021"));
	dialog->add("runner", "when i was a pretty boy, gey sex fuck", nite::Color("#3740c0"));
	dialog->add("betsy", "lol it was you all along", nite::Color("#d20021"));
	dialog->add("runner", "WAKE UP SHEEPLE. DONT YOU SEE?", nite::Color("#3740c0"));

	dialog->start(nite::Vec2(0.0f), 720, 3);

	while(nite::isRunning()){	
		nite::viewUpdate();
		nite::inputUpdate();

		if(nite::keyboardPressed(nite::keySPACE)){
			dialog->cont();
		}


		world.step();
		world.render();

		battle->step();
		battle->render();

		dialog->step();
		dialog->render();

		nite::graphicsRender();
	}
	nite::graphicsEnd();


	return 0;
}