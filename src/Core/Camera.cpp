#include "Camera.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"
static bool cameraFreeroam = false;
static nite::Console::CreateProxy cpAnDatTo("cl_camera_freeroam", nite::Console::ProxyType::Bool, sizeof(bool), &cameraFreeroam);

/////////////
// COMMAND: cl_camera_follow
////////////
static nite::Console::Result cfCameraFollow(Vector<String> params){
	// static auto game = Game::getGameCoreInstance();
	if(params.size() < 1){
		return nite::Console::Result("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto &_id = params[0];
	if(!nite::isNumber(_id)){
		return nite::Console::Result("'"+_id+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto id = nite::toInt(_id);
	// if(!game->world.exists(id)){
	// 	nite::Console::add("Entity id '"+_id+"' does not exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	// 	return;		
	// }
	cameraFreeroam = false;
	// game->camera.follow(id);	
}
static auto cfCameraFollowIns = nite::Console::CreateFunction("cl_camera_follow", &cfCameraFollow); 

/////////////
// COMMAND: cl_camera_snap
////////////
static nite::Console::Result cfCameraSnap(Vector<String> params){
	// static auto game = Game::getGameCoreInstance();
	if(params.size() < 1){
		return nite::Console::Result("Not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
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
	followId = -1;
}

void Game::Camera::update(nite::Vec2 &v, float mu){
	// static auto *inst = Game::getGameCoreInstance();		
	// nite::setView(true, nite::RenderTargetGame);
	// nite::Vec2 P = nite::getView(nite::RenderTargetGame);
	// nite::Vec2 K = v - nite::getSize() * 0.5f;
	// P.lerp(K, mu);
	// setViewPosition(P, nite::RenderTargetGame);
}

void Game::Camera::update(nite::Vec2 &v){
	update(v, 0.15f);
}

void Game::Camera::update(){
	// static auto *inst = Game::getGameCoreInstance();	
	// if(cameraFreeroam){
	// 	nite::Vec2 np = nite::getView(nite::RenderTargetGame);
	// 	if(nite::keyboardCheck(nite::keyA)){
	// 		np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(32.0f, 0.0f));
	// 	}
	// 	if(nite::keyboardCheck(nite::keyD)){
	// 		np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(32.0f, 0.0f));
	// 	}		
	// 	if(nite::keyboardCheck(nite::keyW)){
	// 		np.set(nite::getView(nite::RenderTargetGame) - nite::Vec2(0.0f, 32.0f));			
	// 	}
	// 	if(nite::keyboardCheck(nite::keyS)){
	// 		np.set(nite::getView(nite::RenderTargetGame) + nite::Vec2(0.0f, 32.0f));						
	// 	}		
	// 	nite::setView(true, nite::RenderTargetGame);
	// 	nite::setViewPosition(np, nite::RenderTargetGame);
	// 	return;
	// }
	// if(followId == -1 || !inst->world.exists(followId)) return;
	// update(inst->world.objects[followId]->position);
}

void Game::Camera::follow(int id){
	this->followId = id;
	cameraFreeroam = false;
	nite::print("following entity id "+nite::toStr(id));
	// setView(id != -1, nite::RenderTargetGame);
}