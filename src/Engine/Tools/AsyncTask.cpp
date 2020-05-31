#include "Tools.hpp"
#include <pthread.h> 

static pthread_mutex_t asyncTaskMutex;
static pthread_mutex_t uniqueIdMutex;
static int getUniqueId(){
	pthread_mutex_lock(&uniqueIdMutex);
	static int seed = 1 + nite::randomInt(847, 955);
	int id = ++seed;
	pthread_mutex_unlock(&uniqueIdMutex);
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
	auto task = Shared<nite::AsyncTask::Context>(new nite::AsyncTask::Context());
	task->lambda = lambda;
	task->delayTime = 0;
	task->start();
	pthread_mutex_lock(&asyncTaskMutex);
	pool.push_back(task);
	pthread_mutex_unlock(&asyncTaskMutex);
	return task;
}

Shared<nite::AsyncTask::Context> nite::AsyncTask::spawn(nite::AsyncTask::ALambda lambda, nite::SmallPacket &payload){
	auto task = Shared<nite::AsyncTask::Context>(new nite::AsyncTask::Context());
	task->lambda = lambda;
	task->delayTime = 0;
	task->payload = payload;
	task->start();
	pthread_mutex_lock(&asyncTaskMutex);
	pool.push_back(task);
	pthread_mutex_unlock(&asyncTaskMutex);
	return task;
}

Shared<nite::AsyncTask::Context> nite::AsyncTask::spawn(nite::AsyncTask::ALambda lambda, UInt64 delayTime){
	auto task = Shared<nite::AsyncTask::Context>(new nite::AsyncTask::Context());
	task->lambda = lambda;
	task->delayTime = delayTime;
	task->start();
	pthread_mutex_lock(&asyncTaskMutex);
	pool.push_back(task);
	pthread_mutex_unlock(&asyncTaskMutex);
	return task;
}

Shared<nite::AsyncTask::Context> nite::AsyncTask::spawn(nite::AsyncTask::ALambda lambda, UInt64 delayTime, nite::SmallPacket &payload){
	auto task = Shared<nite::AsyncTask::Context>(new nite::AsyncTask::Context());
	task->lambda = lambda;
	task->delayTime = delayTime;
	task->payload = payload;
	task->start();
	pthread_mutex_lock(&asyncTaskMutex);
	pool.push_back(task);
	pthread_mutex_unlock(&asyncTaskMutex);
	return task;
}

void nite::AsyncTask::update(){
	pthread_mutex_lock(&asyncTaskMutex);
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
	pthread_mutex_unlock(&asyncTaskMutex);
}

void  nite::AsyncTask::end(){
	pthread_mutex_lock(&asyncTaskMutex);
	for(int i = 0; i < pool.size(); ++i){
		auto task = pool[i].get();
		task->stop();
	}	
	pthread_mutex_unlock(&asyncTaskMutex);
}