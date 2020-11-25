#include <cmath>

#include "../Engine/Graphics.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"
#include "Camera.hpp"
#include "Client.hpp"
#include "../Entity/Base.hpp"
#include "../Game.hpp"
static bool cameraFreeroam = false;
static nite::Console::CreateProxy cpAnDatTo("cl_camera_freeroam", nite::Console::ProxyType::Bool, sizeof(bool), &cameraFreeroam);

static bool cameraForceZoom = true;
static nite::Console::CreateProxy cpCamFZoom("cl_camera_forcezoom", nite::Console::ProxyType::Bool, sizeof(bool), &cameraForceZoom);

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

void Game::Camera::update(nite::Vec2 &towards, float mu){		
	nite::Vec2 v = towards;
	nite::setView(true, nite::RenderTargetGame);
	nite::Vec2 p = nite::getView(nite::RenderTargetGame);
	np = v - nite::getSize() * 0.5f;
	// auto dist = nite::distance(p + nite::getSize() * 0.5f, v);
	// float xdist = nite::abs((p.x + nite::getWidth() * 0.5f)-v.x);
	// float ydist = nite::abs((p.y + nite::getHeight() * 0.5f)-v.y);
	// auto threshold = nite::Vec2(384.0f, 192.0f);
	// auto offset = threshold;
	// if(nite::mouseCheck(nite::butMIDDLE)){
	// 	v = nite::getView(nite::RenderTargetGame) + nite::mousePosition();
	// 	threshold = nite::Vec2(0.0f);
	// 	offset = threshold;
	// }	
	// if(xdist > threshold.x){
	// 	float xbias = nite::getSign(v.x - (p.x + nite::getWidth() * 0.5f));
	// 	np = (v - nite::getSize() * 0.5f) + nite::Vec2(offset.x * xbias, 0.0f);
	// }
	// if(ydist > threshold.y){
	// 	float ybias = nite::getSign(v.y - (p.y + nite::getHeight() * 0.5f));
	// 	np = (v - nite::getSize() * 0.5f) + nite::Vec2(0.0f, offset.y * ybias);
	// }	
	// if(std::isnan(v.x) || std::isnan(v.y)){
	// 	return;
	// }
	p.lerpDiscrete(np, mu);
	setViewPosition(p, nite::RenderTargetGame);	
}

void Game::Camera::update(nite::Vec2 &v){
	update(v, 0.08f);
}

void Game::Camera::update(){	
	if(cameraFreeroam){
		nite::Vec2 np = nite::getView(nite::RenderTargetGame);
		nite::Vec2 lnp = np;
		if(nite::keyboardCheck(nite::keyLEFT)){
			np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(64.0f, 0.0f));
		}
		if(nite::keyboardCheck(nite::keyRIGHT)){
			np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(64.0f, 0.0f));
		}		
		if(nite::keyboardCheck(nite::keyUP)){
			np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(0.0f, 64.0f));			
		}
		if(nite::keyboardCheck(nite::keyDOWN)){
			np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(0.0f, 64.0f));						
		}		
		lnp.lerpDiscrete(np, 0.35f);
		nite::setView(true, nite::RenderTargetGame);
		nite::setViewPosition(lnp, nite::RenderTargetGame);
		return;
	}
	if(followId == 0 || client == NULL){
		return;
	}
	if(cameraForceZoom){
		nite::setZoom(nite::RenderTargetGame, 1.0f);
	}
	auto ent = client->getEntity(followId);
	if(ent != NULL){
		update(ent->position);
	}
}

void Game::Camera::follow(UInt16 id){
	this->followId = id;
	cameraFreeroam = false;
	nite::print("following entity id "+nite::toStr(id));
	nite::setView(id != 0, nite::RenderTargetGame);
}