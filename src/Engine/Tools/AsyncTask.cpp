#include "Tools.hpp"

static int getUniqueId(){
	static int seed = 1 + nite::randomInt(847, 955);
	return ++seed;
}

static Vector<Shared<nite::AsyncTask::Context>> pool;

void nite::AsyncTask::Context::pause(){
	if(state == nite::AsyncTask::State::Killed){
		return;
	}
	state =  nite::AsyncTask::State::Paused;
}

void nite::AsyncTask::Context::resume(){
	if(state != nite::AsyncTask::State::Killed){
		return;
	}	
	state = nite::AsyncTask::State::Running;
}

void nite::AsyncTask::Context::start(){
	if(state == nite::AsyncTask::State::Idle){
		state = nite::AsyncTask::State::Running;
		startTime = nite::getTicks();
	}	
}

void nite::AsyncTask::Context::stop(){
	if(state == nite::AsyncTask::State::Killed){
		return;
	}	
	state = nite::AsyncTask::State::Killed;
}

nite::AsyncTask::Context::Context(){
	state = nite::AsyncTask::State::Idle;
	id = getUniqueId();
	lambda = [](nite::AsyncTask::Context &context){
		return;
	};
}

void nite::AsyncTask::Context::step(){
	if(state == nite::AsyncTask::State::Running && nite::getTicks()-startTime > delayTime){
		lambda(*this);
	}
}

Shared<nite::AsyncTask::Context> nite::AsyncTask::spawn(nite::AsyncTask::ALambda lambda){
	return spawn(lambda, 0);
}

Shared<nite::AsyncTask::Context> nite::AsyncTask::spawn(nite::AsyncTask::ALambda lambda, UInt64 delayTime){
	auto task = Shared<nite::AsyncTask::Context>(new nite::AsyncTask::Context());
	task->lambda = lambda;
	task->delayTime = delayTime;
	task->start();
	pool.push_back(task);
	return task;
}

void nite::AsyncTask::update(){
	for(int i = 0; i < pool.size(); ++i){
		auto task = pool[i].get();
		if(task->state == nite::AsyncTask::State::Killed){
			pool.erase(pool.begin() + i);
			--i;
		}
	}
	for(int i = 0; i < pool.size(); ++i){
		auto task = pool[i].get();
		task->step();
	}
}

void  nite::AsyncTask::end(){
	for(int i = 0; i < pool.size(); ++i){
		auto task = pool[i].get();
		task->stop();
	}	
}