#include <stdlib.h>
#include <string.h>
#include "Network.hpp"
#include "Types.hpp"
#include "Tools/Tools.hpp"
#include "Packets.hpp"

nite::Packet::Packet(UInt16 Header){
	clear();
	setHeader(Header);
}

nite::Packet::Packet(){
	clear();
}

nite::Packet::~Packet(){

}

void nite::Packet::copy(const Packet &other){ 
	maxSize = other.maxSize;
	memcpy(this->data, other.data, nite::NetworkMaxPacketSize);
	reset();
}

nite::Packet& nite::Packet::operator= (const nite::Packet &other){
	copy(other);
	return *this;
}

void nite::Packet::clear(){
	memset(data, '0', nite::NetworkMaxPacketSize);
	index = nite::NetworkHeaderSize + nite::NetworkOrderSize + nite::NetworkAckSize; // starts from the send byte mark
	maxSize = index;
	setOrder(0);
}

size_t nite::Packet::getSize(){
	return maxSize - (nite::NetworkHeaderSize + nite::NetworkOrderSize + nite::NetworkAckSize);
}

void nite::Packet::setHeader(UInt16 header){
	memcpy(data, &header, nite::NetworkHeaderSize);
}

UInt16 nite::Packet::getHeader(){
	UInt16 header;
	memcpy(&header, data, nite::NetworkHeaderSize);
	return header;
}

UInt32 nite::Packet::getOrder(){
	UInt32 order;
	memcpy(&order, data + nite::NetworkHeaderSize, nite::NetworkOrderSize);
	return order;
}

void nite::Packet::setOrder(UInt32 order){
	memcpy(data + nite::NetworkHeaderSize, &order, nite::NetworkOrderSize);
}

UInt32 nite::Packet::getAck(){
	UInt32 ack;
	memcpy(&ack, data + nite::NetworkHeaderSize + nite::NetworkOrderSize, nite::NetworkAckSize);
	return ack;
}

void nite::Packet::setAck(UInt32 ack){
	memcpy(data + nite::NetworkHeaderSize + nite::NetworkOrderSize, &ack, nite::NetworkAckSize);
}

void nite::Packet::reset(){
	index = nite::NetworkHeaderSize + nite::NetworkOrderSize + nite::NetworkAckSize; 
}

void nite::Packet::setIndex(size_t index){
	this->index = index;
}

bool nite::Packet::write(const String str){
	if((index >= nite::NetworkMaxPacketSize) || (index + str.length() + 1 > nite::NetworkMaxPacketSize)){
		nite::print("failed to write to packet: too big");
		return false;
	}
	size_t sl = str.length() + 1;
	memcpy(data + index, str.c_str(), sl);
	maxSize += sl; 
	index += sl;
	return true;
}

bool nite::Packet::read(String &str){
	if(index >= nite::NetworkMaxPacketSize){
		return false;
	}
	for(size_t i = index; i < nite::NetworkMaxPacketSize; ++i){
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

bool nite::Packet::write(const void *data, size_t size){
	if(size == 0){
		return false;
	}
	if((index >= nite::NetworkMaxPacketSize) || (index + size > nite::NetworkMaxPacketSize)){
		nite::print("failed to write to packet: too big");
		return false;
	}
	memcpy(this->data + index, data, size);
	maxSize += size; 
	index += size;
	return true;
}

bool nite::Packet::read(void *data, size_t size){
	if((index >= nite::NetworkMaxPacketSize) || (index + size > nite::NetworkMaxPacketSize)){
		return false;
	}
	memcpy(data, this->data + index, size);
	index += size;
	return true;
}

