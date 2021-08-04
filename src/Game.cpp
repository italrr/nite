#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"
#include "Engine/Network.hpp"
#include "Engine/Shapes.hpp"

#include "Game.hpp"
#include "World.hpp"


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


static Shared<Game::GameCore> currentGame = std::make_shared<Game::GameCore>(Game::GameCore());

Game::GameCore::GameCore(){

}

void Game::GameCore::init(){
	nite::graphicsInit();
	nite::inputInit();
	dialog = Shared<Game::DialogBox>(new Game::DialogBox());
	battle = Shared<Game::Battle>(new Game::Battle());	

	dialog->add("betsy", "this is a line of text", nite::Color("#d20021"));
	dialog->add("runner", "when i was a pretty boy, gey sex fuck", nite::Color("#3740c0"));
	dialog->add("betsy", "lol it was you all along", nite::Color("#d20021"));
	dialog->add("runner", "WAKE UP SHEEPLE. DONT YOU SEE?", nite::Color("#3740c0"));

	dialog->start(nite::Vec2(0.0f), 720, 3);	
}

void Game::GameCore::step(){
	nite::viewUpdate();
	nite::inputUpdate();	
	battle->step();
	dialog->step();
	if(nite::keyboardPressed(nite::keySPACE)){
		dialog->cont();
	}

}

void Game::GameCore::render(){
	dialog->render();
	battle->render();
	nite::graphicsRender();
}


Shared<Game::GameCore> Game::getGameIns(){
	return currentGame;
}


int main(int argc, char* argv[]){
	
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);


	auto game = Game::getGameIns();

	game->init();


	auto player = std::make_shared<Player>(Player());
	auto mob = std::make_shared<Mob>(Mob());

	Game::World world;
	world.init(1000, 1000);
	world.add(player);
	world.add(mob);
	

	// battle->start({player}, {mob});
	// battle->setDialog(dialog);



	while(nite::isRunning()){	

		world.step();
		world.render();

		game->step();
		game->render();

	}
	nite::graphicsEnd();


	return 0;
}