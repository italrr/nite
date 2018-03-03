#include <stdlib.h>
#include <string.h>
#include "Network.hpp"
#include "Types.hpp"
#include "Tools/Tools.hpp"
#include "Packets.hpp"

using namespace nite;

void Packet::setHeader(UInt16 Header){
	memcpy(Data, &Header, sizeof(UInt16));
	return;
}

UInt16 Packet::getHeader(){
	UInt16 p;
	memcpy(&p, Data, sizeof(UInt16));
	return p;
}

unsigned Packet::getSize(){
	return index;
}

void Packet::Reset(){
	index = sizeof(UInt16);
}

void Packet::setIndex(unsigned i){
	index = i;
}

unsigned Packet::getIndex(){
	return index;
}

bool Packet::writeString(String str){
	if(index >= MAX_PACKET_SIZE || index+str.length()+1>MAX_PACKET_SIZE){
		return false;
	}
	unsigned s = str.length();
	static char c[1] = {'\0'};
	memcpy(Data+index, str.c_str(), s);
	index += s;
	memcpy(Data+index, c, 1);
	index++;
	return true;
}

bool Packet::readString(String &str){
	if(index >= MAX_PACKET_SIZE){
		return false;
	}
	unsigned end 	= 0;
	unsigned Si 	= 0;
	for(unsigned i=index; i<MAX_PACKET_SIZE; i++){
		Si++;
		if (Data[i] == '\0'){
			end = i;
			break;
		}
	}
	if (!end){
		return false;
	}
	char Buf[Si];
	memcpy(Buf, Data+index, Si);
	str = Buf;
	index += Si;
	return true;
}

bool Packet::Write(void *data, unsigned Si){
	if(index >= MAX_PACKET_SIZE || index+Si > MAX_PACKET_SIZE){
		return false;
	}
	memcpy(Data+index, data, Si);
	index += Si;
	return true;
}

bool Packet::Read(void *data, unsigned Si){
	if(index >= MAX_PACKET_SIZE || index+Si>MAX_PACKET_SIZE){
		return false;
	}
	memcpy(data, Data+index, Si);
	index += Si;
	return true;
}

void Packet::clear(){
	memset(Data, '0', MAX_PACKET_SIZE);
	index	= sizeof(UInt16);
}

Packet::Packet(UInt16 Header){
	memset(Data, '0', MAX_PACKET_SIZE);
	index	= sizeof(UInt16);
	Max 	= MAX_PACKET_SIZE;
	setHeader(Header);
}

Packet::Packet(){
	memset(Data, '0', MAX_PACKET_SIZE);
	index	= sizeof(UInt16);
	Max 	= MAX_PACKET_SIZE;
}

unsigned Packet::getMax(){
	return Max;
}

Packet::~Packet(){

}
