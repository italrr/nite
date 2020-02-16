#ifndef NITE_NETWORK_DATA_PACKETS_HPP
	#define NITE_NETWORK_DATA_PACKETS_HPP

	namespace nite {

		static const size_t NetworkHeaderSize = sizeof(UInt16);
		static const size_t NetworkOrderSize = sizeof(UInt32);
		static const size_t NetworkMaxPacketSize = NetworkHeaderSize + NetworkOrderSize + 1024 * 2; //  2 kilobytes + 2 bytes for the header + 4 bytes for packet order

		struct Packet {
				size_t index;
				size_t maxSize;
				char data[nite::NetworkMaxPacketSize];
				Packet(UInt16 header);
				Packet();
				Packet(UInt32 order);
				~Packet();				
				UInt32 getOrder();
				void setOrder(UInt32 order);
				UInt16 getHeader();
				void setHeader(UInt16 header);
				void copy(const Packet &other);
				void reset();
				void clear();
				void setIndex(size_t index);
				bool write(const void *data, size_t Si);
				bool read(String &str);
				bool read(void *data, size_t Si);
				bool write(const String str);
				nite::Packet& operator= (const nite::Packet &other);

		};
	}
#endif
