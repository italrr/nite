#ifndef NITE_NETWORK_HPP
	#define NITE_NETWORK_HPP

	#include "Types.hpp"

	namespace nite {

		const size_t MAX_PACKET_SIZE 						= 2048;
		const size_t MAX_TRANSPORT_PACKET_SIZE 	= 10240;

		void socketInit();
		void socketEnd();
		class IP_Port {
			public:
				IP_Port();
				IP_Port(String _IP, unsigned _Port);
				String IP;
				unsigned Port;
				String str();
		};
		/*
		================
		UDP
		================
		*/
		class UDPSocket	{
			private:
				int socketId;
				bool opened;
				bool nonBlocking;
			public:
				UDPSocket();
				~UDPSocket();
				bool open(unsigned Port);
				void close();
				bool isOpen();
				bool setNonBlocking(bool m);
				bool sendData(IP_Port &Dest, char *Data, unsigned size);
				int receiveData(IP_Port &Sender, char *Data, unsigned size);
		};
		/*
		================
		TCP
		================
		*/
		class TCPSocket {
			private:
				int socketId;
				bool opened;
				bool connected;
				bool listening;
				bool nonBlocking;
			public:
				TCPSocket();
				~TCPSocket();
				bool openSocket(unsigned Port);
				void close();
				bool isOpen();
				bool isListening();
				bool isConnected();
				void disconnect();
				bool setNonBlocking(bool m);
				bool setListen(unsigned Backlog);
				int acceptClient(IP_Port &Who);
				bool connectIP(const IP_Port &IP);
				bool sendData(int Sock, char *Data, unsigned size);
				bool sendData(char *Data, unsigned size);
				int receiveData(int Sock, char *Data, unsigned size);
				int receiveData(char *Data, unsigned size);
		};
		void socketClose(int s);
	}

#endif
