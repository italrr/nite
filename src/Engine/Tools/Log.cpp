#include "Tools.hpp"
#include <iostream>
#include <pthread.h>
#include "../Console.hpp"

static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

void nite::printInit(){
    if(pthread_mutex_init(&count_mutex, NULL) != 0){ 
        nite::print("failed to start mutex");
        nite::exit();
    }  
}

void nite::printEnd(){
	pthread_mutex_destroy(&count_mutex);
}

void nite::print(const String &Message){
	pthread_mutex_lock(&count_mutex);
	String Timestamp = nite::getTimestamp();
	nite::Console::add(Message, false);
	std::cout << "[" << Timestamp << "] " << Message << std::endl;
	pthread_mutex_unlock(&count_mutex);
}

void nite::print(int n){
	nite::print(nite::toStr(n));
}

void nite::print(unsigned n){
	nite::print(nite::toStr(n));
}

void nite::print(float n){
	nite::print(nite::toStr(n));
}

void nite::print(double n){
	nite::print(nite::toStr(n));
}

void nite::print(const nite::Vec2 &v){
	nite::print("{"+nite::toStr(v.x)+", "+nite::toStr(v.y)+"}");
}

void nite::print(bool n){
	nite::print(nite::toStr(n));
}

void nite::print(char *c){
	nite::print(String(c));
}

void nite::print(const char *c){
		nite::print(String(c));
}

void nite::print(UInt64 n){
	nite::print(nite::toStr((long long unsigned int)n));
}

void nite::print(Int64 n){
	nite::print(nite::toStr((long long unsigned int)n));
}

// void nite::print(size_t n){
// 	nite::print(nite::toStr((long long unsigned int)n));
// }
