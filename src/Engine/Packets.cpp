#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <WS2tcpip.h>
#else
#include <stdio.h>
#include <errno.h>
	#include <errno.h> 
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <sys/types.h> 
	#include <netinet/in.h>
	#include <fcntl.h>
#endif

#include "Tools/Tools.hpp"
#include "Network.hpp"
#include "Packets.hpp"
#include "Types.hpp"
#define String std::string
#define UInt16 	uint16_t
#define UInt32 	uint32_t

nite::IP_Port::IP_Port(const String &ip, UInt16 port){
	set(ip, port);
}

nite::IP_Port::IP_Port(){
	set("127.0.0.1", NetworkDefaultPort);
}

nite::IP_Port::IP_Port(const nite::IP_Port &ip, UInt16 nport){
	set(ip.ip, nport);
}

bool nite::IP_Port::operator== (const IP_Port &other){
	return isSame(other);
}

bool nite::IP_Port::isSame(const IP_Port &other){
	return this->address == other.address && this->port == other.port;
}

void nite::IP_Port::set(const String &ip, UInt16 port){
	this->port = port;
	this->ip = ip;
	this->address = inet_addr(ip.c_str());
}

bool nite::IP_Port::isBlack(){
	return address == 0 && port == 0;
}

void nite::IP_Port::clear(){
	address = 0;
	port = 0;
}

String nite::IP_Port::str() const{
	return (String)*this;
}

nite::IP_Port::operator std::string() const {
	return ip +":"+ toStr(port);
}

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

void nite::Packet::	clear(){
	memset(data, '0', nite::NetworkMaxPacketSize);
	index = nite::NetworkHeaderSize + nite::NetworkOrderSize + NetworkAckSize; // starts from the send byte mark
	maxSize = index;
}

size_t nite::Packet::getSize(){
	return maxSize - (nite::NetworkHeaderSize + nite::NetworkOrderSize + NetworkAckSize);
}

void nite::Packet::setHeader(UInt16 header){
	memcpy(data, &header, nite::NetworkHeaderSize);
}

UInt16 nite::Packet::getHeader() const{
	UInt16 header;
	memcpy(&header, data, nite::NetworkHeaderSize);
	return header;
}

UInt32 nite::Packet::getOrder() const {
	UInt32 order;
	memcpy(&order, data + nite::NetworkHeaderSize, nite::NetworkOrderSize);
	return order;
}

void nite::Packet::setOrder(UInt32 order){
	memcpy(data + nite::NetworkHeaderSize, &order, nite::NetworkOrderSize);
}

UInt32 nite::Packet::getAck() const {
	UInt32 ack;
	memcpy(&ack, data + nite::NetworkHeaderSize + nite::NetworkOrderSize, nite::NetworkAckSize);
	return ack;
}

void nite::Packet::setAck(UInt32 ack){
	memcpy(data + nite::NetworkHeaderSize + nite::NetworkOrderSize, &ack, nite::NetworkAckSize);
}


void nite::Packet::reset(){
	index = nite::NetworkHeaderSize + nite::NetworkOrderSize + NetworkAckSize; 
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

bool nite::Packet::read(nite::Vec2 &v){
	bool x = false, y = false;
	x = this->read(&v.x, sizeof(v.x));
	y = this->read(&v.y, sizeof(v.y));	
	return x && y;
}

bool nite::Packet::write(const nite::Vec2  &v){
	bool x = false, y = false;
	x = this->write(&v.x, sizeof(v.x));
	y = this->write(&v.y, sizeof(v.y));	
	return x && y;
}