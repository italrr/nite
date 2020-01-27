#include "Tools.hpp"

nite::StepTimer::StepTimer(String name){
	this->name = name;
}

nite::StepTimer::StepTimer(){
	name = "StepTimer";
}

void nite::StepTimer::init(){
	lastTick = nite::getTicks();
}

void nite::StepTimer::end(){
	time = nite::getTicks() - lastTick;
}

String nite::StepTimer::getStatus(){
	return "["+name+"] Elapsed "+nite::toStr(time)+" msecs";
}