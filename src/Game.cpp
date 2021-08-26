#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"
#include "Engine/Network.hpp"
#include "Engine/Shapes.hpp"

#include "Game.hpp"


struct Player : Game::Entity {

	void create(){
		setPosition(nite::Vec2(200, 500));
		setMass(1.0f);
		this->friction = 4.0f;
		this->nickname = "Player";
		entityType = Game::EntityType::PLAYER;
	}
	
	void step(){
		
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
		this->nickname = "Jeremy";
		entityType = Game::EntityType::MOB;
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


struct Mob2 : Game::Entity {

	void create(){
		setPosition(nite::Vec2(500, 500));
		setMass(1.0f);
		this->friction = 4.0f;
		this->nickname = "Billy";
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
	// init engine
	nite::graphicsInit();
	nite::inputInit();

	// init game core
	dialog = Shared<Game::DialogBox>(new Game::DialogBox());
	battle = Shared<Game::Battle>(new Game::Battle());	
	world = Shared<Game::World>(new Game::World());	

	// start game
	player = std::make_shared<Player>(Player());
	player->setupStat(5);
	player->addBaseStat(BaseStatType::Strength, 1000);
	
	mob = std::make_shared<Mob>(Mob());
	mob->setupStat(5);
	mob->addBaseStat(BaseStatType::Strength, 1000);

	mob2 = std::make_shared<Mob2>(Mob2());
	player->loadAnim();
	mob->loadAnim();
	mob2->loadAnim();
	world->init(1000, 1000);
	world->add(player);
	world->add(mob);
	world->add(mob2);


}

void Game::GameCore::step(){
	nite::viewUpdate();
	nite::inputUpdate();	
	world->step();
	battle->step();
	dialog->step();

	// player movement
	if(player.get() != NULL){
		if(!dialog->isShowing() && !battle->isShowing()){
			if(nite::keyboardCheck(nite::keyW)){
				player->moveEntity(0.0f, -1.0f);
			}
			if(nite::keyboardCheck(nite::keyD)){
				player->moveEntity(1.0f, 0.0f);
			}
			if(nite::keyboardCheck(nite::keyS)){
				player->moveEntity(0.0f, 1.0f);
			}
			if(nite::keyboardCheck(nite::keyA)){
				player->moveEntity(-1.0f, 0.0f);
			}	
		}		
		if(nite::keyboardPressed(nite::keyZ)){
			if(!battle->isShowing()){
				battle->start({player}, {mob}); // this could go so wrong lol
			}
			// if(dialog->isReady() && !dialog->isShowing()){
			// 	dialog->reset();
			// 	dialog->add("betsy", "this is a line of text", nite::Color("#d20021"));
			// 	dialog->add("runner", "when i was a pretty boy, gey sex fuck", nite::Color("#3740c0"));
			// 	dialog->add("betsy", "lol it was you all along", nite::Color("#d20021"));
			// 	dialog->add("runner", "WAKE UP SHEEPLE. DONT YOU SEE?", nite::Color("#3740c0"));
			// 	dialog->start(nite::Vec2(0.0f), 720, 3);
			// }else{
			// 	dialog->cont();
			// }
		}		
	}
}

void Game::GameCore::render(){
	world->render();	
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

	while(nite::isRunning()){	
		game->step();
		game->render();

	}
	nite::graphicsEnd();


	return 0;
}