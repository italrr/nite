#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <WS2tcpip.h>
#else
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
#endif

#include "Tools/Tools.hpp"
#include "Network.hpp"

static size_t Rx = 0;
static size_t Sx = 0;

static void close_(int s){
	#ifdef _WIN32
		closesocket(s);
	#else
		close(s);
	#endif
}

void nite::socketInit(){
	#ifdef _WIN32
		WSADATA WsaData;
		if(WSAStartup(MAKEWORD(2,2), &WsaData) != 0){
			nite::print("failed to initialize Winsock"); // TODO: add proper error handling 
		}
	#endif
}

void nite::socketEnd(){
	#ifdef _WIN32
		WSACleanup();
	#endif
}

nite::IP_Port::IP_Port(const String &ip, UInt16 port){
	set(ip, port);
}

nite::IP_Port::IP_Port(){
	set("127.0.0.1", NetworkDefaultPort);
}

bool nite::IP_Port::operator== (IP_Port &other){
	return isSame(other);
}

bool nite::IP_Port::isSame(IP_Port &other){
	return this->address == other.address && this->port == other.port;
}

void nite::IP_Port::set(const String &ip, UInt16 port){
	this->port = port;
	this->ip = ip;
	this->address = inet_addr(ip.c_str());
}

String nite::IP_Port::str(){
	return (String)*this;
}

nite::IP_Port::operator std::string() const {
	return ip +":"+ toStr(port);
}

nite::UDPSocket::UDPSocket(){
	opened = false;
	nonBlocking = false;
}

nite::UDPSocket::~UDPSocket(){
	if (opened){
		close_(sock);
	}
}

Int32 nite::UDPSocket::getSock(){
	return sock;
}

bool nite::UDPSocket::open(UInt16 port){
	this->port = port;
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET){
		nite::print("failed to create UDP socket"); // TODO: add proper error handling
		return false;
	}
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(sock, (const sockaddr*) &addr, sizeof(sockaddr_in)) == SOCKET_ERROR){
 		nite::print("failed to bind UDP socket to port "+nite::toStr(port)); // TODO: add proper error handling
		close_(sock);
		return false;
	}
	opened = true;
	nite::print("UDP socket opened at "+nite::toStr(port)+" ("+nite::toStr(sock)+")");
	return true;
}

void nite::UDPSocket::close(){
	if (opened){
		close_(sock);
		opened = false;
		nite::print("UDP socket closed at "+nite::toStr(port)+" ("+nite::toStr(sock)+")");

	}
}

bool nite::UDPSocket::isOpen(){
	return opened;
}

bool nite::UDPSocket::setNonBlocking(bool m){
	if (m == nonBlocking){
		return false;
	}
	#ifdef _WIN32
		u_long b = m ? 1 : 0;
		if (ioctlsocket(sock, FIONBIO, &b) != 0){
			nonBlocking = m; // TODO: add proper error handling
			return true;
		}
	#else
		if (fcntl(socketId, F_SETFL, O_NONBLOCK, m) == -1){
			nonBlocking = m; // TODO: add proper error handling
			return true;
		}
	#endif
	return false;
}

size_t nite::UDPSocket::send(const IP_Port &dest, char *data, size_t size){
	if (!opened){
		return false;
	}
	sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(dest.port);
	addr.sin_addr.s_addr = dest.address;
	ssize_t s = sendto(sock, data, size, 0, (sockaddr*)&addr, sizeof(sockaddr_in)); // TODO: add proper error handling
	if(s > 0){
		Sx += s;
	}
	return s;
}

size_t nite::UDPSocket::send(const IP_Port &dest, nite::Packet &packet){
	return send(dest, packet.data, packet.maxSize);
}

size_t nite::UDPSocket::recv(IP_Port &sender, char *buffer){
	sockaddr_in from;
	socklen_t as = sizeof(sockaddr);
	ssize_t s = recvfrom(sock, buffer, NetworkMaxPacketSize, 0, (sockaddr*)&from, &as); // TODO: add proper error handling
	sender.address = from.sin_addr.s_addr;
	sender.port = ntohs(from.sin_port);
	// TODO: pass ip to sender
	if(s > 0){
		Sx += s;
	}
	return s;
}
	
size_t nite::UDPSocket::recv(IP_Port &sender, nite::Packet &buffer){
	buffer.reset();
	size_t s = recv(sender, buffer.data);
	buffer.maxSize = s;
	return s;
}
