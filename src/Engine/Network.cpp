#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Network.hpp"
#include "Tools/Tools.hpp"

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

using namespace nite;

static unsigned Rx = 0;
static unsigned Sx = 0;

static void close_(int s){
	#ifdef _WIN32
		closesocket(s);
	#else
		close(s);
	#endif
}

/*
================
init socket
================
*/
void nite::socketInit(){
	#ifdef _WIN32
		WSADATA WsaData;
		WSAStartup(MAKEWORD(2,2), &WsaData) == NO_ERROR;
	#endif
}

/*
================
end socket
================
*/
void nite::socketEnd(){
	#ifdef _WIN32
		WSACleanup();
	#endif
}


/*
================
close a socket manually
================
*/
void nite::socketClose(int s){
	close_(s);
	return;
}

/*
================
IP_Port constructor with IP and Port as arguments
================
*/
IP_Port::IP_Port(String _IP, unsigned _Port){
	IP		= _IP;
	Port	= _Port;
}

/*
================
IP_Port constructor empty
================
*/
IP_Port::IP_Port(){
	IP		= "127.0.0.1";
	Port 	= 0;
}

/*
================
IP_Port Get formatted as String
================
*/
String IP_Port::str(){
	return IP +":"+ toStr(Port);
}

/*
================
UDPSocket
================
*/

/* Constructor */
UDPSocket::UDPSocket(){
	opened = 0;
	nonBlocking = 0;
}

/* destructor */
UDPSocket::~UDPSocket(){
	if (opened){
		close_(socketId);
	}
}

/* open a handling socket */
bool UDPSocket::open(unsigned Port){
	if((socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		return 0;
	}
	if (socketId <= 0){
		return 0;
	}
	sockaddr_in Address;
	Address.sin_family = AF_INET;
	Address.sin_addr.s_addr = INADDR_ANY;
	Address.sin_port = htons(Port);
	if (bind(socketId, (const sockaddr*) &Address, sizeof(sockaddr_in)) < 0){
		close_(socketId);
		return 0;
	}
	opened = 1;
	return 1;
}

/* close the handling sockets */
void UDPSocket::close(){
	if (opened){
		close_(socketId);
		opened = 0;
	}
}

/* Check if there's a handling socket open */
bool UDPSocket::isOpen(){
	return opened;
}

/* set socket as nonblocking(true/false) */
bool UDPSocket::setNonBlocking(bool m){
	if (m == nonBlocking){
		return 1;
	}
	#ifdef _WIN32
		DWORD _nonBlocking = m;
		if (ioctlsocket(socketId, FIONBIO, &_nonBlocking) != 0 ){
			nonBlocking = m;
			return 0;
		}
	#else
		if (fcntl(socketId, F_SETFL, O_NONBLOCK, m) == -1){
			nonBlocking = m;
			return 0;
		}
	#endif
	return 1;
}

static void strToAddr(String IP_, sockaddr_in &Address){
	unsigned a,b,c,d;
	Vector<String> Comp;
	nite::strSplit(IP_, ".", Comp);
	a = nite::toInt(Comp[0]);
	b = nite::toInt(Comp[1]);
	c = nite::toInt(Comp[2]);
	d = nite::toInt(Comp[3]);
	unsigned addr = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
	Address.sin_addr.s_addr = htonl(addr);
}

/* send data to IP */
bool UDPSocket::sendData(IP_Port &Dest, char *Data, unsigned size){
	if (opened != 1){
		return 0;
	}
	int Sent;
	sockaddr_in Address;
    Address.sin_family = AF_INET;
    Address.sin_port = htons( Dest.Port );
	strToAddr(Dest.IP, Address);
	Sent = sendto(socketId, Data, size, 0, (sockaddr*)&Address, sizeof(sockaddr_in));
	if (Sent != size){
		return 0;
	}
	return 1;
}

/* Recieve data from handling socket */
int UDPSocket::receiveData(IP_Port &Sender, char *Data, unsigned size){
	sockaddr_in From;
	socklen_t fromLength = sizeof(From);
	int S;
	S = recvfrom(socketId, Data, MAX_PACKET_SIZE, 0, (sockaddr*)&From, &fromLength);
	Sender.IP = ntohl(From.sin_addr.s_addr);
	Sender.Port = ntohs(From.sin_port);
	return S;
}

/*
================
TCPSocket
================
*/

/* Constructor */
TCPSocket::TCPSocket(){
	socketId = -1;
	opened = 0;
	connected = 0;
	connected = 0;
	nonBlocking = 0;
}
//#include "Log.h"
//#include "../Tools.h"
//#include <errno.h>
/* Destructor  */
TCPSocket::~TCPSocket(){
	if (opened or connected){
		connected = 0;
		opened = 0;
		connected = 0;
		close_(socketId);
	}
}

/* set socket as nonblocking(true/false) */
bool TCPSocket::setNonBlocking(bool m){
	if (m == nonBlocking){
		return 1;
	}
	#ifdef _WIN32
		DWORD _nonBlocking = m;
		if (!(ioctlsocket(socketId, FIONBIO, &_nonBlocking) != 0)){
			nonBlocking = m;
			return 0;
		}
	#else
		if (fcntl(socketId, F_SETFL, O_NONBLOCK, m) == -1){
			nonBlocking = m;
			return 0;
		}
	#endif
	return 1;
}

/* Recieve data from the specified socket. Server function. */
int  TCPSocket::receiveData(int Sock, char *Data, unsigned size){
	#ifdef _WIN32
		return recv(Sock, Data, size, 0);
	#else
		return recv(Sock, Data, size, (!nonBlocking ? MSG_DONTWAIT : 0));
	#endif

}

/* Recieve data from the handling socket. Client function. */
int  TCPSocket::receiveData(char *Data, unsigned size){
	#ifdef _WIN32
		return recv(socketId, Data, size, 0);
	#else
		return recv(socketId, Data, size, (!nonBlocking ? MSG_DONTWAIT : 0));
	#endif
}

/* send data to the specified socket. Server function. */
bool TCPSocket::sendData(int Sock, char *Data, unsigned size){
	if(!opened){
		return 0;
	}
	if (send(Sock, Data, size, 0)<0){
		return 0;
	}
	return 1;
}

/* send data to the handling socket. Client function. */
bool TCPSocket::sendData(char *Data, unsigned size){
	if(!connected){
		return 0;
	}
	if (send(socketId, Data, size, 0)<0){
		return 0;
	}
	return 1;
}

/* accept an incoming connection from the handling socket. Server function. */
int TCPSocket::acceptClient(IP_Port &Who){
	if(!connected){
		return -1;
	}
	sockaddr_in Address;
	socklen_t size = sizeof(Address);
	memset(&Address, '0', sizeof(sockaddr_in));
	int s = accept(socketId, (sockaddr*)&Address, &size);
	#ifdef _WIN32
		Who.IP = inet_ntoa(Address.sin_addr);
	#else
		char buf[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &Address.sin_addr, buf, INET_ADDRSTRLEN);
		Who.IP = buf;
	#endif
	Who.Port = ntohs(Address.sin_port);
	if (s >= 0){
		#ifdef _WIN32
			DWORD _nonBlocking = nonBlocking;
			ioctlsocket(s, FIONBIO, &_nonBlocking);
		#else
			fcntl(s, F_SETFL, O_NONBLOCK, nonBlocking);
		#endif

	}
	return s;
}

/* set the handling socket to listen. Server function. */
bool TCPSocket::setListen(unsigned Backlog){
	if (!opened or connected){
		return 0;
	}
	listen(socketId, Backlog);
	connected = 1;
	return 1;
}

/* open the handling socket. Server function. */
bool TCPSocket::openSocket(unsigned Port){
	if (connected){
		return 0;
	}
	if((socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return 0;
	}
	sockaddr_in Address;
	memset(&Address, '0', sizeof(Address));
    Address.sin_family = AF_INET;
    Address.sin_addr.s_addr = htonl(INADDR_ANY);
    Address.sin_port = htons(Port);
    bind(socketId, (struct sockaddr*)&Address, sizeof(sockaddr_in));
    opened = 1;
    return 1;
}

/* Connect to an IP. Client function. */
bool TCPSocket::connectIP(const IP_Port &IP){
	if (opened or connected){
		return 0;
	}
	if (socketId != -1){
		close_(socketId);
	}
	if((socketId = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return 0;
	}
	sockaddr_in Address;
	memset(&Address, '0', sizeof(Address));
    Address.sin_family = AF_INET;
    Address.sin_port = htons(IP.Port);
	strToAddr(IP.IP, Address);
	if(connect(socketId, (sockaddr*)&Address, sizeof(Address)) < 0){
		//nite::Log(nite::toStr(errno));
		return 0;
	}
	connected = 1;
	return 1;
}

/* Check if the handling socket is open. Server function */
bool TCPSocket::isOpen(){
	return opened;
}

/* Check if the handling socket is connected. Client function */
bool TCPSocket::isConnected(){
	return connected;
}

/* Check if the handling socket is listening. Server function */
bool TCPSocket::isListening(){
	return connected;
}

/* close down any communication from the handling socket. Function for both roles. */
void TCPSocket::close(){
	if (opened){
		connected = 0;
		opened = 0;
		connected = 0;
		close_(socketId);
	}
}

/* close down any communication from the handling socket. Function for both roles. */
void TCPSocket::disconnect(){
	if (connected){
		connected = 0;
		opened = 0;
		connected = 0;
		close_(socketId);
	}
}
