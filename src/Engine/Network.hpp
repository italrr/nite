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

		class UDPSocket	{
			private:
				Int32 sock;
				UInt16 port;
				bool opened;
				bool nonBlocking;
			public:
				UDPSocket();
				~UDPSocket();
				bool open(UInt16 port);
				void close();
				bool isOpen();
				bool setNonBlocking(bool m);
				bool send(const IP_Port &dest, char *data, size_t size);
				bool send(const IP_Port &dest, nite::Packet &packet);
				bool recv(IP_Port &sender, char *buffer);
				bool recv(IP_Port &sender, nite::Packet &buffer);
		};

	}

#endif
