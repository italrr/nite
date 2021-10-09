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
		this->animOverworld->load("data/entity/human_class.json");
		this->setAnim(Game::OverworldAnimType::RUN_DOWN, 0);
		updateStandAnim();
    	reshape(nite::Vec2(43, 125));
	}
	
	void step(){
		stepAnim();
	}	

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(nite::Color("#503335", 1.0f));
		nite::setDepth(-lerpPos.y);
		this->renderOverworld();
	}
};


struct Mob : Game::Entity {

	void create(){
		setPosition(nite::Vec2(500, 500));
		setMass(1.0f);
		this->friction = 4.0f;
		this->nickname = "Jeremy";
		entityType = Game::EntityType::MOB;
		this->animOverworld->load("data/entity/human_class.json");
		this->setAnim(Game::OverworldAnimType::RUN_DOWN, 0);
		updateStandAnim();		
		reshape(nite::Vec2(43, 125));
	}
	
	void step(){
		stepAnim();
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setColor(nite::Color("#ecbcb4", 1.0f));
		nite::setDepth(-lerpPos.y);
		this->renderOverworld();
	}
};


struct Mob2 : Game::Entity {

	void create(){
		setPosition(nite::Vec2(500, 500));
		setMass(1.0f);
		this->friction = 4.0f;
		this->nickname = "Billy";
		entityType = Game::EntityType::MOB;
		this->animOverworld->load("data/entity/human_class.json");
		this->setAnim(Game::OverworldAnimType::RUN_DOWN, 0);
		reshape(nite::Vec2(43, 125));
		updateStandAnim();		
	}
	
	void step(){
		stepAnim();		
	}

	void render(){
		nite::setRenderTarget(nite::RenderTargetGame);
		nite::setDepth(-lerpPos.y);
		nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
		this->renderOverworld();
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
	optionsMenu = Shared<Game::UIListMenu>(new Game::UIListMenu());
	dialog = Shared<Game::DialogBox>(new Game::DialogBox());
	battle = Shared<Game::Battle>(new Game::Battle());
	world = Shared<Game::World>(new Game::World());
	storyLine = Shared<Game::Story::StoryLine>(new Game::Story::StoryLine());
	
	// manually setup options menu
    auto theme = std::make_shared<Game::UITheming>(Game::UITheming());
    theme->load("data/ui/base_theme.json");
	optionsMenu->theme = theme;
	optionsMenu->onCreate();


	storyLine->setup(optionsMenu, dialog);


	// start game
	player = std::make_shared<Player>(Player());
	player->setupStat(100);
	player->addBaseStat(BaseStatType::Dexterity, 50);
	player->addBaseStat(BaseStatType::Agility, 100);
	player->addBaseStat(BaseStatType::Endurance, 100);
	// player->addBaseStat(BaseStatType::Endurance, 10000);
	player->fullHeal();
	player->printInfo();
	
	mob = std::make_shared<Mob>(Mob());
	mob->setupStat(100);
	mob->addBaseStat(BaseStatType::Endurance, 100);
	mob->addBaseStat(BaseStatType::Luck, 100);
	mob->fullHeal();

	mob2 = std::make_shared<Mob2>(Mob2());
	mob2->setupStat(5);
	mob2->addBaseStat(BaseStatType::Strength, 20);

	player->loadAnim();
	mob->loadAnim();
	mob2->loadAnim();
	world->init(1000, 1000);
	world->add(player);
	world->add(mob);
	world->add(mob2);

	world->setFollowObject(true, player->symRefId);


}

void Game::GameCore::step(){
	nite::viewUpdate();
	nite::inputUpdate();	
	world->step();
	battle->step();
	optionsMenu->step();
	dialog->update();
	storyLine->step();



	// player movement
	if(player.get() != NULL){
		if(!dialog->isShowing() && !battle->isShowing() && !storyLine->interDevice->busy){
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
			// if(!battle->isShowing()){
			// 	battle->start({player}, {mob}); // this could go so wrong lol
			// }

			// greetings
			if(!storyLine->interDevice->busy){
				Vector<Shared<DialogLine>> greetingsLine;
				greetingsLine.push_back(Game::buildLine("Kid", "My father suffers from the L5 condition"));
				greetingsLine.push_back(Game::buildLine(mob->animOverworld, "Kid", "Could you help me find the medicine?"));
				

				Vector<Shared<DialogLine>> rejectLine;
				rejectLine.push_back(Game::buildLine(mob->animOverworld,"Kid", "Wow, you're a terrible person."));
				auto rejectFirst = std::make_shared<Game::Story::InteractionTreeExposition>(Game::Story::InteractionTreeExposition());
				rejectFirst->lines = rejectLine;


				Vector<Shared<DialogLine>> approvalLine;
				approvalLine.push_back(Game::buildLine(mob->animOverworld,"Kid", "Thanks mister, I really appreciate it!"));				
				auto approvalFirst = std::make_shared<Game::Story::InteractionTreeExposition>(Game::Story::InteractionTreeExposition());
				approvalFirst->lines = approvalLine;								


				auto requestFirst = std::make_shared<Game::Story::InteractionTreeDialog>(Game::Story::InteractionTreeDialog());
				Vector<Shared<Game::Story::InteractionTreeDialogOption>> requestFirstOptions;
				requestFirstOptions.push_back(Game::Story::buildTreeDialogOption("Yes", approvalFirst->symRefId));
				requestFirstOptions.push_back(Game::Story::buildTreeDialogOption("No", rejectFirst->symRefId));
				requestFirst->options = requestFirstOptions;
				requestFirst->title = "Could you help me find the medicine?";



				auto greetingsFirst = std::make_shared<Game::Story::InteractionTreeExposition>(Game::Story::InteractionTreeExposition());
				greetingsFirst->lines = greetingsLine;
				greetingsFirst->next.push_back(requestFirst->symRefId);

				auto greetingsStart = std::make_shared<Game::Story::InteractionTreeContact>(Game::Story::InteractionTreeContact());
				greetingsStart->next.push_back(greetingsFirst->symRefId);
				
				storyLine->interDevice->addInter(approvalFirst);
				storyLine->interDevice->addInter(rejectFirst);
				storyLine->interDevice->addInter(requestFirst);
				storyLine->interDevice->addInter(greetingsStart);
				storyLine->interDevice->addInter(greetingsFirst);
				storyLine->interDevice->start();
			}
				

			// if(dialog->isReady() && !dialog->isShowing()){
			// 	dialog->reset();
			// 	dialog->add("betsy", "Can I ask you a question?", nite::Color("#d20021"));
			// 	dialog->add("runner", "when i was a pretty boy, gey sex fuck", nite::aColor("#3740c0"));
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
    nite::setDepth(nite::DepthTop);
    nite::setRenderTarget(nite::RenderTargetUI);  	
	optionsMenu->render();
	battle->render();
	storyLine->render();
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