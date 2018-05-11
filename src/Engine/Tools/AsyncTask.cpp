#include "Tools.hpp"

static int getUniqueId(){
	static int seed = 1 + nite::randomInt(847, 955);
	return ++seed;
}

static Vector<Shared<nite::AsyncTask*>> pool;
static Vector<int]> stopQueue;

		
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
	task = [](nite::AsyncTask &context){
		return;
	};
}

Shared<nite::AsyncTask*> nite::spawnAsyncTask(AsyncLambda lambda){
	auto task = Shared<nite::AsyncTask*>(new AsyncTask());
	
}

void nite::updateAsyncTask(){

}

void nite::stopAsyncTask(){


}