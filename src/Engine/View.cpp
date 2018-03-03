#include "View.hpp"
#include "Graphics.hpp"
#include "Tools/Tools.hpp"

struct View {
	nite::Vec2 position;
	bool enable;
	View(){
		enable = false;
	}
};

static View views[RenderTargetNumber];

static unsigned shakeScreenTarget;
static UInt64 shakeScreenTimeout = 0;
static float shakeScreenMagnitude;
static UInt64 lastShakeScreen = 0;

void nite::viewUpdate(){
	if(nite::getTicks()-lastShakeScreen > shakeScreenTimeout) return;
	nite::Vec2 &target = views[shakeScreenTarget].position;
	target.x += 1.25f * ((nite::randomInt(1, 2) == 1.0f) ? -1.0f : 1.0f);
	target.y += 1.25f * ((nite::randomInt(1, 2) == 1.0f) ? -1.0f : 1.0f);
}

void nite::shakeScreen(int target, float magnitude, UInt64 timeout){
	shakeScreenTimeout = timeout;
	shakeScreenMagnitude = magnitude;
	lastShakeScreen = nite::getTicks();
	shakeScreenTarget = target;
}

nite::Vec2 nite::getView(unsigned target){
	return views[target].enable ? views[target].position : nite::Vec2();
}

float nite::getViewX(unsigned target){
	return views[target].enable ? views[target].position.x : 0;
}

float nite::getViewY(unsigned target){
	return views[target].enable ? views[target].position.y : 0;
}

void nite::setViewPosition(float x, float y, unsigned target){
	views[target].position.x = views[target].enable ? x : 0;
	views[target].position.y = views[target].enable ? y : 0;
}

void nite::setViewPosition(const nite::Vec2 &P, unsigned target){
	views[target].position.set(views[target].enable ? P : nite::Vec2());
}

void nite::setView(bool m, unsigned target){
	views[target].enable = m;
}
void nite::setViewPosition(float x, float y){

}

void nite::setViewPosition(const nite::Vec2 &P){

}
