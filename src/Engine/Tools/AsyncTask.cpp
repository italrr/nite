#include "Tools.hpp"

static int getUniqueId(){
	static int seed = 1 + nite::randomInt(847, 955);
	return ++seed;
}

static Vector<Shared<nite::AsyncTask>> pool;
		
int nite::AsyncTask::getStatus(){
	return status;
}

void nite::AsyncTask::pause(){
	if(status == nite::AsyncTaskStatus::Killed){
		return;
	}
	status =  nite::AsyncTaskStatus::Paused;
}

void nite::AsyncTask::resume(){
	if(status != nite::AsyncTaskStatus::Killed){
		return;
	}	
	status = nite::AsyncTaskStatus::Running;
}

void nite::AsyncTask::start(){
	if(status == nite::AsyncTaskStatus::Idle){
		status = nite::AsyncTaskStatus::Running;
	}	
}

void nite::AsyncTask::stop(){
	if(status == nite::AsyncTaskStatus::Killed){
		return;
	}	
	status = nite::AsyncTaskStatus::Killed;
}

nite::AsyncTask::AsyncTask(){
	status = nite::AsyncTaskStatus::Idle;
	id = getUniqueId();
	lambda = [](const nite::AsyncTask &context){
		return;
	};
}

Shared<nite::AsyncTask> nite::AsyncTask::spawn(nite::AsyncLambda lambda){
	auto task = Shared<nite::AsyncTask>(new nite::AsyncTask());
	task->lambda = lambda;
	task->start();
	pool.push_back(task);
	return task;
}


void nite::AsyncTask::step(){
	if(status == nite::AsyncTaskStatus::Running){
		lambda(*this);
	}
}

void nite::updateAsyncTask(){
	for(int i = 0; i < pool.size(); ++i){
		auto task = pool[i].get();
		if(task->getStatus() == nite::AsyncTaskStatus::Killed){
			pool.erase(pool.begin() + i);
			--i;
		}
	}
	for(int i = 0; i < pool.size(); ++i){
		auto task = pool[i].get();
		task->step();
	}
}

void nite::stopAsyncTask(){
	for(int i = 0; i < pool.size(); ++i){
		auto task = pool[i].get();
		task->stop();
	}	
}