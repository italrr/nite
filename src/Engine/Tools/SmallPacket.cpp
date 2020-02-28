#include <stdlib.h>
#include <string.h>
#include "Tools.hpp"


nite::SmallPacket::SmallPacket(){
	clear();
}

nite::SmallPacket::~SmallPacket(){

}

void nite::SmallPacket::copy(const nite::SmallPacket &other){ 
	memcpy(this->data, other.data, NITE_SMALLPACKET_SIZE);
	reset();
}

nite::SmallPacket& nite::SmallPacket::operator= (const nite::SmallPacket &other){
	copy(other);
}

void nite::SmallPacket::clear(){
	memset(data, '0', NITE_SMALLPACKET_SIZE);
	index = 0; 
}

void nite::SmallPacket::reset(){
	index = 0; 
}

void nite::SmallPacket::setIndex(size_t index){
	this->index = index;
}

bool nite::SmallPacket::write(const String str){
	if((index >= NITE_SMALLPACKET_SIZE) || (index + str.length() + 1 > NITE_SMALLPACKET_SIZE)){
		nite::print("failed to write to SmallPacket: too big");
		return false;
	}
	size_t sl = str.length() + 1;
	memcpy(data + index, str.c_str(), sl);
	index += sl;
	return true;
}

bool nite::SmallPacket::read(String &str){
	if(index >= NITE_SMALLPACKET_SIZE){
		return false;
	}
	for(size_t i = index; i < NITE_SMALLPACKET_SIZE; ++i){
		if (this->data[i] == '\0'){
			size_t size = i - index; // don't include the nullterminated
			char buff[size];
			memcpy(buff, data + index, size);
			str = String(buff, size);
			index = i + 1;
			return true;
		}
	}
	return false;
}

bool nite::SmallPacket::write(const void *data, size_t size){
	if((index >= NITE_SMALLPACKET_SIZE) || (index + size > NITE_SMALLPACKET_SIZE)){
		nite::print("failed to write to packet: too big");
		return false;
	}
	memcpy(this->data + index, data, size);
	index += size;
	return true;
}

bool nite::SmallPacket::read(void *data, size_t size){
	if((index >= NITE_SMALLPACKET_SIZE) || (index + size > NITE_SMALLPACKET_SIZE)){
		return false;
	}
	memcpy(data, this->data + index, size);
	index += size;
	return true;
}

