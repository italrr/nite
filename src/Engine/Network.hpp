#ifndef NITE_NETWORK_HPP
	#define NITE_NETWORK_HPP

	#include "Types.hpp"
	#include "Packets.hpp"

	namespace nite {
		static const UInt16 NetworkDefaultPort = 14101; 

		void socketInit();
		void socketEnd();

		struct IP_Port {
			IP_Port();
			IP_Port(const String &ip, UInt16 port);
			String ip;
			UInt16 port;
			Int32 address;
			String str();
			void set(const String &ip, UInt16 port);
			operator std::string() const;
		};

		struct UDPSocket	{
			private:
				Int32 sock;
				UInt16 port;
				bool opened;
				bool nonBlocking;
			public:
				Int32 getSock();
				UDPSocket();
				~UDPSocket();
				bool open(UInt16 port);
				void close();
				bool isOpen();
				bool setNonBlocking(bool m);
				size_t send(const IP_Port &dest, char *data, size_t size);
				size_t send(const IP_Port &dest, nite::Packet &packet);
				size_t recv(IP_Port &sender, char *buffer);
				size_t recv(IP_Port &sender, nite::Packet &buffer);
		};

	}

#endif
