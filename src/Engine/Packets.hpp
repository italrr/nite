#ifndef NITE_NETWORK_DATA_PACKETS
	#define NITE_NETWORK_DATA_PACKETS

	/*
		A nite data packet is simple two bytes for the header and the rest of
		the packet.
		The whole packet can't be bigger than nite's network stack buffer, which
		by default is 10240 bytes.
	*/

	namespace nite {

		class Packet {
			private:
				unsigned Max;
				unsigned index;
			public:
				char Data[MAX_PACKET_SIZE];
				unsigned getSize();
				unsigned getMax();
				Packet(UInt16 Header);
				Packet();
				~Packet();
				UInt16 getHeader();
				void setHeader(UInt16 Header);
				void Reset();
				void clear();
				void setIndex(unsigned i);
				unsigned getIndex();
				bool Write(void *data, unsigned Si);
				bool readString(String &str);
				bool Read(void *data, unsigned Si);
				bool writeString(String str);

		};
	}
#endif
