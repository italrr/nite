#include "Game.hpp"
#include "Sword.hpp"
#include "Engine/Shapes.hpp"

using namespace nite;

static Game::GameMaster *instance = NULL;

Game::Camera::Camera(){
	followId = -1;
}

void Game::Camera::update(nite::Vec2 &v, float mu){
	static auto *inst = Game::getInstance();		
	nite::setView(true, nite::RenderTargetGame);
  nite::Vec2 P = nite::getView(nite::RenderTargetGame);
  nite::Vec2 K = v - nite::getSize() * 0.5f;
  P.lerp(K, mu);
  setViewPosition(P, nite::RenderTargetGame);
}

void Game::Camera::update(nite::Vec2 &v){
	update(v, 0.15f);
}

void Game::Camera::update(){
	static auto *inst = Game::getInstance();	
	if(followId == -1 || !inst->world.exists(followId)) return;
	update(inst->world.objects[followId]->position);
}

void Game::Camera::follow(int id){
	this->followId = id;
	// setView(id != -1, nite::RenderTargetGame);
}

// Player Info
static void cfReloadPlayerAnims(Vector<String> params){
	static auto game = Game::getInstance();
	game->player->entityReloadAnimation();
}
static auto cfReloadPlayerAnimsIns = nite::Console::CreateFunction("player_reloadanims", &cfReloadPlayerAnims);  


static void cfPrintPlayerInfo(Vector<String> params){
	static auto game = Game::getInstance();
	game->player->printEntity();
}
static auto cfPrintPlayerInfoIns = nite::Console::CreateFunction("player_info", &cfPrintPlayerInfo);  

static void cfPlayerStatUp(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 2){
		nite::Console::add("Not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &stat = params[0];
	String &amount = params[1];
	if(!nite::isNumber(amount)){
		nite::Console::add("'"+amount+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}	
	int amnt = nite::toInt(amount);
	int added = 0;
	bool success = true;
	if(stat == "str"){
		added = game->player->addBaseStat(Game::BaseStatType::Strength, amnt);
	}else
	if(stat == "agi"){
		added = game->player->addBaseStat(Game::BaseStatType::Agility, amnt);
	}else
	if(stat == "dex"){
		added = game->player->addBaseStat(Game::BaseStatType::Dexterity, amnt);
	}else	
	if(stat == "end"){
		added = game->player->addBaseStat(Game::BaseStatType::Endurance, amnt);
	}else	
	if(stat == "luk"){
		added = game->player->addBaseStat(Game::BaseStatType::Luck, amnt);
	}else	
	if(stat == "int"){
		added = game->player->addBaseStat(Game::BaseStatType::Intelligence, amnt);
	}else{
		success = false;
		nite::print("Invalid stat "+stat);
	}
	if(success){
		nite::print("Increase player's "+stat+" by "+nite::toStr(added));
	}
	
}
static auto cfPlayerStatUpIns = nite::Console::CreateFunction("player_statadd", &cfPlayerStatUp);  

static void cfPlayerStatReset(Vector<String> params){
	static auto game = Game::getInstance();
	if(params.size() < 1){
		nite::Console::add("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
		return;
	}
	String &stat = params[0];
	bool success = true;
	if(stat == "str"){
		game->player->resetStat(Game::BaseStatType::Strength);
	}else
	if(stat == "agi"){
		game->player->resetStat(Game::BaseStatType::Agility);
	}else
	if(stat == "dex"){
		game->player->resetStat(Game::BaseStatType::Dexterity);
	}else	
	if(stat == "end"){
		game->player->resetStat(Game::BaseStatType::Endurance);
	}else	
	if(stat == "luk"){
		game->player->resetStat(Game::BaseStatType::Luck);
	}else	
	if(stat == "int"){
		game->player->resetStat(Game::BaseStatType::Intelligence);
	}else{
		success = false;
		nite::print("Invalid stat "+stat);
	}
	if(success){
		nite::print("Reset player's "+stat+" to 0");
	}
	
}
static auto cfPlayerStatResetIns = nite::Console::CreateFunction("player_statreset", &cfPlayerStatReset); 

void Game::GameMaster::start(){
	instance = this;
	nite::graphicsInit();
	nite::inputInit();
	renderEntities = true;
	map.load("data/map/test.json", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
	
	// auto window = Shared<nite::WindowUI>(new nite::WindowUI());

	// // Button1	
	// auto panel1 = new nite::PanelUI();
	// auto button1a = new nite::ButtonUI("Button 1a", [](const nite::ButtonUI &button){
	// 	nite::print("pressed button 1a");
	// 	return;
	// });
	// auto button1b = new nite::ButtonUI("Button 1b", [](const nite::ButtonUI &button){
	// 	nite::print("pressed button 1b");
	// 	return;
	// });
	// auto button1c = new nite::ButtonUI("Button 1c", [](const nite::ButtonUI &button){
	// 	nite::print("pressed button 1c");
	// 	return;
	// });		

	// panel1->setLayout(Shared<nite::LayoutSystemUI>(new nite::Layout::HBox()));		
	// // panel1->add(Shared<nite::BaseUIComponent>(new nite::PanelUI()));
	// panel1->add(Shared<nite::BaseUIComponent>(button1a));
	// panel1->add(Shared<nite::BaseUIComponent>(button1b));
	// panel1->add(Shared<nite::BaseUIComponent>(button1c));
	// // panel1->add(Shared<nite::BaseUIComponent>(new nite::PanelUI()));
	// window->add(Shared<nite::BaseUIComponent>(panel1));

	// auto button2 = new nite::ButtonUI("Button 2", [](const nite::ButtonUI &button){
	// 	nite::print("pressed button 2");
	// 	return;
	// });
	// // button2->flex = 1.0f;	
	// window->add(Shared<nite::BaseUIComponent>(button2));

	// auto button3 = new nite::ButtonUI("Button 3", [](const nite::ButtonUI &button){
	// 	nite::print("pressed button 3");
	// 	return;
	// });
	// // button3->flex = 1.0f;		
	// window->add(Shared<nite::BaseUIComponent>(button3));	
	// // // Text
	// // window->add(Shared<nite::BaseUIComponent>(new nite::TextUI("Glyph metrics are, as the name suggests, certain distances associated with each glyph that describe how to position this glyph while creating a text layout.")));
	// // // Checkbox
	// // auto checkbox = Shared<nite::BaseUIComponent>(new nite::CheckboxUI("Checkbox", [](bool v, const nite::CheckboxUI &checkbox){
	// // 	nite::print("state changed "+nite::toStr(v));
	// // 	return;
	// // }));
	// // window->add(checkbox);
	// // static_cast<nite::CheckboxUI*>(checkbox.get())->setMargin(nite::Vec2(64.0f));

	// //set layout and add to ui engine
	// window->setLayout(Shared<nite::LayoutSystemUI>(new nite::Layout::VBox()));
	// window->setSize(nite::Vec2(900, 400));
	// ui.add(window);

	Dict<String, nite::ListenerLambda> listeners;
	listeners["method1"] = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component){
		nite::print("pressed");
		return;
	};
	listeners["checkboxchange1"] = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component){
		auto *ref = static_cast<nite::ListenerInfoCheckbox*>(info.get());
		nite::print(ref->value);
		return;
	};
	listeners["hovermethod"] = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component){
		auto *ref = static_cast<nite::PanelUI*>(&component);
		ref->setBackgroundColor(nite::Color(0.0f, 1.0f, 0.0f, 1.0f));
		return;
	};
	listeners["unhovermethod"] = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component){
		auto *ref = static_cast<nite::PanelUI*>(&component);
		ref->setBackgroundColor(nite::Color(0.0f, 0.0f, 1.0f, 1.0f));
		return;
	};	

	listeners["onclicktext"] = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component){
		nite::print("onclicktext");
		return;
	};
			
	// ui.build("ui.json", listeners);

	isRunning = true;
	auto player = Shared<nite::PhysicsObject>(new Game::Player());
	world.add(player);

	this->player = static_cast<Entity*>(player.get());
	this->player->fullHeal();
	
	auto mob = Shared<nite::PhysicsObject>(new Game::BasicMob());
	world.add(mob);
	mob->position.set(1100, 400);
	auto mobEntity = static_cast<Entity*>(mob.get());
	mobEntity->fullHeal();	
	

	auto sword = Shared<Game::BaseItem>(new Sword());
	sword.get()->load("data/weap/base_sword.json");
	auto playerEntity = static_cast<Entity*>(player.get());
	playerEntity->addItem(sword);	
	playerEntity->setActiveItem(Game::InventoryActiveSlot::Main, sword);	
	camera.follow(player->id);
}

void Game::GameMaster::update(){
	// nite::setZoom(nite::RenderTargetGame, 0.80f);
	// Basic Shortcuts
	if(nite::keyboardPressed(nite::keyF1)){
		world.debugPhysics = !world.debugPhysics;
	}
	if(nite::keyboardPressed(nite::keyTILDE)){
		if(nite::Console::isOpen()){
			nite::Console::close();
		}else{
			nite::Console::open();
		}
	}

	// Update World
	nite::viewUpdate();
	nite::inputUpdate();

	nite::Console::update();
	ui.update();
	map.update();
	camera.update();
	world.update();
	world.step();
}

void Game::GameMaster::render(){	
	nite::graphicsUpdate();
	nite::Console::render();
	ui.render();
	map.render();
	world.render();		
}

void Game::GameMaster::end(){
	isRunning = false;
}

void Game::GameMaster::onEnd(){

}

Game::GameMaster *Game::getInstance(){
	return instance;
}

int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);

	Game::GameMaster game;
	game.start();

	// float angle = 0.0;

	// String conjucture = " Vincent Berihuete ";
	// float sepAngle = 360.0f / ((float)conjucture.length());
	// nite::Font font("data/font/monaco.ttf", 16);

	while(game.isRunning){

		// nite::setDepth(0);
		// nite::setRenderTarget(nite::RenderTargetUI);
		// nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
		
		// for(int z = 0; z < conjucture.length(); ++z){
		// 	float _an = -z * sepAngle;
		// 	nite::Vec2 _rp(nite::getSize());
		// 	float x = nite::getWidth() * 0.5f + 250 * nite::cos(nite::toRadians(_an));
		// 	float y = nite::getHeight() * 0.5f + 250 * nite::sin(nite::toRadians(_an));
		// 	float fromAn = nite::toDegrees(nite::arctan(y, x));
		// 	String cc = conjucture.substr(z, 1);
		// 	font.draw(cc, x, y, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f);
			
		// 	// if(z > 0){
		// 		_an = -(z - 1) * sepAngle;
		// 		float x2 = nite::getWidth() * 0.5f + 250 * nite::cos(nite::toRadians(_an));
		// 		float y2 = nite::getHeight() * 0.5f + 250 * nite::sin(nite::toRadians(_an));				
		// 		nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
		// 		nite::Draw::Line(x2, y2, x, y);
		// 	// }
		// }

		game.update();
		game.render();
	}
	game.onEnd();

	return 0;
}
