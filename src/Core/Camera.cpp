#include "../Engine/Graphics.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"
#include "Camera.hpp"
#include "Client.hpp"
#include "../Game.hpp"
static bool cameraFreeroam = false;
static nite::Console::CreateProxy cpAnDatTo("cl_camera_freeroam", nite::Console::ProxyType::Bool, sizeof(bool), &cameraFreeroam);

/////////////
// COMMAND: cl_camera_follow
////////////
static nite::Console::Result cfCameraFollow(Vector<String> params){
	auto game = Game::getGameCoreInstance();
	if(params.size() < 1){
		return nite::Console::Result("mot enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto &_id = params[0];
	if(!nite::isNumber(_id)){
		return nite::Console::Result("'"+_id+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto id = nite::toInt(_id);
	if(!game->client.world.exists(id)){
		return nite::Console::Result("entity id '"+_id+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	cameraFreeroam = false;
	game->client.camera.follow(id);	
	return nite::Console::Result();
}
static auto cfCameraFollowIns = nite::Console::CreateFunction("cl_camera_follow", &cfCameraFollow); 

/////////////
// COMMAND: cl_camera_snap
////////////
static nite::Console::Result cfCameraSnap(Vector<String> params){
	if(params.size() < 1){
		return nite::Console::Result("not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto &_x = params[0];
	auto &_y = params[1];
	if(!nite::isNumber(_x)){
		return nite::Console::Result("'"+_x+"' is not a valid parameter. It must be a number", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	if(!nite::isNumber(_y)){
		return nite::Console::Result("'"+_y+"' is not a valid parameter. It must be a number", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}	
	auto x = nite::toInt(_x);
	auto y = nite::toInt(_y);
	cameraFreeroam = false;
	nite::setViewPosition(nite::Vec2(x, y), nite::RenderTargetGame);
	return nite::Console::Result("camera snapped to "+nite::Vec2(x, y).str(), nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
}
static auto cfCameraSnapIns = nite::Console::CreateFunction("cl_camera_snap", &cfCameraSnap); 



Game::Camera::Camera(){
	followId = 0;
	client = NULL;
}

void Game::Camera::start(Game::Client *client){
    this->client = client;
    this->followId = 0;
}

void Game::Camera::update(nite::Vec2 &v, float mu){		
	nite::setView(true, nite::RenderTargetGame);
	nite::Vec2 p = nite::getView(nite::RenderTargetGame);
	nite::Vec2 k = v - nite::getSize() * 0.5f;
	p.lerpDiscrete(k, mu);
	setViewPosition(p, nite::RenderTargetGame);
}

void Game::Camera::update(nite::Vec2 &v){
	update(v, 0.15f);
}

void Game::Camera::update(){	
	if(cameraFreeroam){
		nite::Vec2 np = nite::getView(nite::RenderTargetGame);
		if(nite::keyboardCheck(nite::keyA)){
			np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(32.0f, 0.0f));
		}
		if(nite::keyboardCheck(nite::keyD)){
			np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(32.0f, 0.0f));
		}		
		if(nite::keyboardCheck(nite::keyW)){
			np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(0.0f, 32.0f));			
		}
		if(nite::keyboardCheck(nite::keyS)){
			np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(0.0f, 32.0f));						
		}		
		nite::setView(true, nite::RenderTargetGame);
		nite::setViewPosition(np, nite::RenderTargetGame);
		return;
	}
	if(followId == 0 || client == NULL){
		return;
	}
	nite::setZoom(nite::RenderTargetGame, client->igmenu.open ? 0.60f : 0.70f);
	auto it = client->world.objects.find(followId);
	if(it != client->world.objects.end()){
		update(it->second->position);
	}
}

void Game::Camera::follow(UInt16 id){
	this->followId = id;
	cameraFreeroam = false;
	nite::print("following entity id "+nite::toStr(id));
	nite::setView(id != 0, nite::RenderTargetGame);
}