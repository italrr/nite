#ifndef NITE_NETWORK_DATA_PACKETS_HPP
	#define NITE_NETWORK_DATA_PACKETS_HPP

	#include "Types.hpp"

	namespace nite {

		static const size_t NetworkHeaderSize = sizeof(UInt16);
		static const size_t NetworkOrderSize = sizeof(UInt32);
		static const size_t NetworkAckSize = sizeof(UInt32);
		static const size_t NetworkMaxHeaderSize = NetworkHeaderSize + NetworkOrderSize + NetworkAckSize;
		static const size_t NetworkMaxPacketSize = 512; // 512 bytes is the maximum size for reliable packet transmission over the internet

		struct IP_Port {
			IP_Port();
			IP_Port(const String &ip, UInt16 port);
			IP_Port(const nite::IP_Port &ip, UInt16 nport);
			bool isBlack();
			void clear();
			String ip;
			UInt16 port;
			Int32 address;
			String str() const;
			void set(const String &ip, UInt16 port);
			operator std::string() const;
			bool operator== (const IP_Port &other);
			bool isSame(const IP_Port &other);
		};

		struct Packet {
			nite::IP_Port receiver;
			nite::IP_Port sender;
			size_t index;
			size_t maxSize;
			char data[nite::NetworkMaxPacketSize];
			Packet(UInt16 header);
			Packet();
			~Packet();
			UInt32 getOrder();
			void setOrder(UInt32 order);
			UInt32 getAck();
			void setAck(UInt32 ack);
			UInt16 getHeader();
			void setHeader(UInt16 header);
			void copy(const Packet &other);
			size_t getSize();
			void reset();
			void clear();
			void setIndex(size_t index);
			bool write(const void *data, size_t Si);
			bool read(String &str);
			bool read(void *data, size_t Si);
			bool write(const String str);
			bool read(nite::Vec2 &v);
			bool write(const nite::Vec2  &v);
			nite::Packet& operator= (const nite::Packet &other);
		};

	}
#endif
