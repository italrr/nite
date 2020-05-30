#ifndef NITE_NETWORK_HPP
	#define NITE_NETWORK_HPP

	#include "Tools/Tools.hpp"
	#include "Packets.hpp"

	namespace nite {
		static const UInt16 NetworkDefaultPort = 14101; 
		static const UInt16 NetworkDefaultFileTransferPort = 14102;		

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
			bool operator== (IP_Port &other);
			bool isSame(IP_Port &other);
		};

		struct UDPSocket {
			Int32 sock;
			UInt16 port;
			bool opened;
			bool nonBlocking;
			UDPSocket();
			~UDPSocket();
			bool open(UInt16 port);
			void close();
			bool setNonBlocking(bool m);
			size_t send(const IP_Port &dest, char *data, size_t size);
			size_t send(const IP_Port &dest, nite::Packet &packet);
			size_t recv(IP_Port &sender, char *buffer);
			size_t recv(IP_Port &sender, nite::Packet &buffer);
		};


		namespace FileTransfer {


			enum FTHeader : UInt16 {
				REQUEST = 0,
				ACCEPT,
				CANCELLED,
				OK,
				CHUNK,
				DONE
			};

			struct IndexedFile {
				String path;
				String hash;
				UInt32 size;
			};			

			struct UDPClient;
			struct FTSession {
				FILE *file;
				nite::FileTransfer::UDPClient *client;
				nite::IP_Port ip;
				nite::FileTransfer::IndexedFile indexed;
				UInt32 index;
				UInt32 lastAdd;
				bool sender;
				UInt64 lastPing;
				UInt64 initTime;
				char buffer[nite::NetworkPacketSize];
				std::function<void(const nite::FileTransfer::IndexedFile &file)> callback;
				String id;
				FTSession(){
					index = 0;
					file = NULL;
					callback = [](const nite::FileTransfer::IndexedFile &file){
						
					};
					ping();
				}
				// these got too long jeez (I dont like doing this, but this situation is an emergency)
				void init(	const nite::FileTransfer::IndexedFile &file, const nite::IP_Port &ip, bool sender,
							nite::FileTransfer::UDPClient *client,
							const std::function<void(const nite::FileTransfer::IndexedFile &file)> &callback){
					this->indexed = file;
					this->ip = ip;
					this->callback = callback;
					this->sender = sender;
					this->client = client;
					this->lastPing = nite::getTicks();
					this->initTime = nite::getTicks();
					this->index = 0;
				}
				void ping(){
					this->lastPing = nite::getTicks();
				}
				UInt64 getLastPing(){
					return nite::getTicks() - lastPing;
				}
				void open(){
					if(file != NULL){
						return;
					}
					ping();
					file = fopen(indexed.path.c_str(), this->sender ? "rb" : "wb");
				}
				void write(char *buffer, UInt32 size){
					open();
					ping();
					index += size;
					fwrite(buffer, size, 1, file);
				}
				void read(char *buffer, UInt32 size){
					open();
					ping();
					index += size;		
					fread(buffer, size, 1, file);
				}
				void seek(UInt32 index){
					fseek(file, index, SEEK_SET); 
				}
				void close(){
					if(file != NULL){
						fclose(file);
						file = NULL;
					}
				}
			};

			struct UDPClient { 
				// do not manipulate this field manually
				Dict<String, FTSession> sessions;
				nite::UDPSocket sock;
				bool listening;
				UInt16 port;
				Dict<String, FileTransfer::IndexedFile> indexed;
				~UDPClient();
				UDPClient();
				void indexDir(const String &path);
				void indexFile(const String &file);
				void listen(UInt16 port);
				void stop(const String &id = "");
				void clear();
				void request(	const nite::IP_Port &client,
								const String &hash,
								const String &output,
								const std::function<void(const IndexedFile &file)> &callback = [](IndexedFile file){ return; },
								bool overwrite = true);
			};	

			void end();		
		}

	}

#endif
