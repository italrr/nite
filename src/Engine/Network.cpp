#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> 

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

nite::IP_Port::IP_Port(const nite::IP_Port &ip, UInt16 nport){
	set(ip.ip, nport);
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
	// handicap for testing
	// if(nite::randomInt(1, 3) == 1){
	// 	return 0;
	// }
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
	sender.ip = inet_ntoa(from.sin_addr);
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


/*
	FILE TRANSFER - UDP
*/
static pthread_mutex_t ftSessionMutex[2]; 
static String generateTFId(){
	pthread_mutex_lock(&ftSessionMutex[0]); 
	static hashidsxx::Hashids hash("NITE_FT_SALT_"+nite::toStr(nite::getTicks()));
	static int seed = nite::randomInt(25, 75);
	String fid = hash.encode({++seed, nite::randomInt(25, 99)}); // i know... this might be random enough though
	pthread_mutex_unlock(&ftSessionMutex[0]);
	return fid;
}
static String initFTSession(const nite::FileTransfer::IndexedFile &file,
							const nite::IP_Port &ip, bool sender,
							nite::FileTransfer::UDPClient *client,
							const nite::FileTransfer::FTCallback &callback,
							String pid = ""){
	pthread_mutex_lock(&ftSessionMutex[1]); 
	if(client->sessions.find(pid) != client->sessions.end()){
		pthread_mutex_unlock(&ftSessionMutex[1]);
		return pid;
	}
	String id = pid.length() == 0 ? generateTFId() : pid;
	nite::FileTransfer::FTSession session;
	session.id = id;
	session.init(file, ip, sender, client, callback);
	session.open();
	client->sessions[id] = session;
	nite::print("FT "+id+" "+String(sender ? "sending" : "requesting")+" file '"+file.hash+"' "+(sender ? "to" : "from")+" "+String(ip)+"...");
	pthread_mutex_unlock(&ftSessionMutex[1]);
	return id;
}
static void killFTSession(const String &id, nite::FileTransfer::UDPClient *client, const String &reason = ""){
	pthread_mutex_lock(&ftSessionMutex[1]); 
	auto it = client->sessions.find(id);
	auto &ref = it->second;
	if(it != client->sessions.end()){
		client->sessions[it->first].close();
		nite::print("FT "+ref.id+" cancelled file '"+ref.indexed.hash+"' "+(ref.sender ? "to" : "from")+" "+String(ref.ip)+": "+reason);
		it->second.callback(it->second.indexed, false);
		client->sessions.erase(it->first);
	}
	pthread_mutex_unlock(&ftSessionMutex[1]);
}
static void finishFTSession(const String &id, nite::FileTransfer::UDPClient *client){
	pthread_mutex_lock(&ftSessionMutex[1]); 
	auto it = client->sessions.find(id);
	if(it != client->sessions.end()){
		it->second.close();
		auto &ref = it->second;
		String check = nite::hashFile(ref.indexed.path);
		bool chksm = check == ref.indexed.hash;		
		if(!ref.sender){
			nite::print("FT "+ref.id+" finished file '"+ref.indexed.hash+"' "+(ref.sender ? "to" : "from")+" "+String(ref.ip)+" saved as '"+ref.indexed.path+"': checksum match: "+(chksm ? "true" : "false"));
		}
		it->second.callback(it->second.indexed, chksm);
		client->sessions.erase(it->first);
	}
	pthread_mutex_unlock(&ftSessionMutex[1]);
}
/*
	Client listening
*/
struct __FTListen {
	nite::FileTransfer::UDPClient *client;
	pthread_t thread;
};
static pthread_mutex_t ftListeningMutex[1];
static void *__FTListeningThread(void *vargp){ 
	auto client = static_cast<nite::FileTransfer::UDPClient*>(vargp);
	if(!client->sock.opened){
		client->sock.open(client->port);
	}
	client->sock.setNonBlocking(true);
	nite::print("FT listening on "+nite::toStr(client->port));
	while(true){
		nite::IP_Port who;
		nite::Packet handler;
		UInt64 initTime = nite::getTicks();
		if(!client->listening){
			break;
		}
		auto n = client->sock.recv(who, handler);
		auto isSession = [&](const String &id){
			pthread_mutex_lock(&ftSessionMutex[1]);
			bool found = client->sessions.find(id) != client->sessions.end();
			pthread_mutex_unlock(&ftSessionMutex[1]);			
			return found;
		};
		auto sendChunk = [&](const nite::IP_Port &who, const String &id, char *buffer, UInt32 size, UInt32 fromIndex){
			nite::Packet chunk;
			chunk.setHeader(nite::FileTransfer::FTHeader::CHUNK);						
			chunk.write(id);
			chunk.write(&size, sizeof(size));
			chunk.write(&fromIndex, sizeof(fromIndex));
			chunk.write(buffer, size);
			client->sock.send(who, chunk);
		};		
		auto sendAccept = [&](const nite::IP_Port &who, const String &id, UInt32 fsize){
			nite::Packet accepted;
			accepted.setHeader(nite::FileTransfer::FTHeader::ACCEPT);					
			accepted.write(id);
			accepted.write(&fsize, sizeof(fsize));
			client->sock.send(who, accepted);
		};
		auto sendCancel= [&](const nite::IP_Port &who, const String &id, const String &reason){
			nite::Packet cancelled;
			cancelled.setHeader(nite::FileTransfer::FTHeader::CANCELLED);
			cancelled.write(id);
			cancelled.write(reason);
			client->sock.send(who, cancelled);
		};
		auto sendOK = [&](const nite::IP_Port &who, const String &id, UInt32 index){
			nite::Packet ok;
			ok.setHeader(nite::FileTransfer::FTHeader::OK);
			ok.write(id);
			ok.write(&index, sizeof(index));
			client->sock.send(who, ok);
		};
		auto sendDone = [&](const nite::IP_Port &who, const String &id){
			nite::Packet done;
			done.setHeader(nite::FileTransfer::FTHeader::DONE);						
			done.write(id);
			client->sock.send(who, done);
		};
		auto sendRequest = [&](const nite::IP_Port &who, const String &id, const String &hash){
			nite::Packet request;
			request.setHeader(nite::FileTransfer::FTHeader::REQUEST);
			request.write(hash);
			request.write(id);
			client->sock.send(who, request);
		};
		// TODO: for security, implement a whitelist for clients 
		if(n > 0){
			switch(handler.getHeader()){
				case nite::FileTransfer::FTHeader::REQUEST: {
					String hash;
					String id;
					handler.read(hash);
					handler.read(id);
					auto it = client->indexed.find(hash);
					if(it == client->indexed.end()){
						sendCancel(who, id, "not found");
					}else{
						auto &indexed = it->second;
						initFTSession(indexed, who, true, client, [](const nite::FileTransfer::IndexedFile &file, bool success){return;}, id);
						sendAccept(who, id, indexed.size);
					}
				} break;				
				case nite::FileTransfer::FTHeader::ACCEPT: {
					String id;
					UInt32 fsize;
					handler.read(id);
					handler.read(&fsize, sizeof(fsize));
					nite::Packet ok;
					UInt32 initInd = 0;
					sendOK(who, id, initInd);
					bool isSesh = isSession(id);
					pthread_mutex_lock(&ftSessionMutex[1]);
					auto it = client->sessions.find(id);
					if(isSesh){
						it->second.indexed.size = fsize;						
						it->second.ping();
					}
					pthread_mutex_unlock(&ftSessionMutex[1]);
				} break;
				case nite::FileTransfer::FTHeader::OK: {
					String id;
					UInt32 index;
					handler.read(id);
					handler.read(&index, sizeof(index));
					bool found = isSession(id);
					if(found){
						pthread_mutex_lock(&ftSessionMutex[1]);
						auto it = client->sessions.find(id);
						auto &ref = it->second;
						ref.ping();
						bool done = false;
						if(index != ref.index){
							ref.index = index;
							ref.seek(ref.index); 
						}
						if(ref.index == ref.indexed.size){
							sendDone(who, id);
							done = true;
						}else	
						if(ref.index != ref.indexed.size){						
							UInt32 n = nite::NetworkPacketSize - id.length() - sizeof(index) - sizeof(index) - 1;
							UInt32 size = ref.index + n > ref.indexed.size ? ref.indexed.size - ref.index : n;
							char buffer[size];
							ref.read(buffer, size);
							sendChunk(who, id, buffer, size, index);
						}							
						pthread_mutex_unlock(&ftSessionMutex[1]);
						if(done){
							finishFTSession(id, client);
						}
					}				
				} break;				
				case nite::FileTransfer::FTHeader::CANCELLED: {
					String id;
					String reason;
					handler.read(id);
					handler.read(reason);
					killFTSession(id, client, reason);
				} break;
				case nite::FileTransfer::FTHeader::CHUNK: {
					String id;
					UInt32 size;
					UInt32 fromIndex;
					handler.read(id);
					handler.read(&size, sizeof(size));
					handler.read(&fromIndex, sizeof(fromIndex));
					char buffer[size];
					handler.read(buffer, size);
					bool found = isSession(id);
					if(found){
						bool done = false;
						pthread_mutex_lock(&ftSessionMutex[1]);
						auto it = client->sessions.find(id);
						auto &ref = it->second;
						ref.ping();
						if(fromIndex == ref.index){
							ref.write(buffer, size);
							done = ref.index == ref.indexed.size;
						}
						pthread_mutex_unlock(&ftSessionMutex[1]);
						if(done){						
							finishFTSession(id, client);
							sendDone(who, id);
						}else{
							sendOK(who, id, ref.index);
						}						
					}					
				} break;
				case nite::FileTransfer::FTHeader::DONE: {
					String id;
					handler.read(id);
					bool found = isSession(id);
					if(found){
						pthread_mutex_lock(&ftSessionMutex[1]);
						auto it = client->sessions.find(id);
						auto &ref = it->second;
						if(ref.index != ref.indexed.size){
							sendOK(who, id, ref.index);
							break;
						}
						pthread_mutex_unlock(&ftSessionMutex[1]);
						finishFTSession(id, client);
						sendDone(who, id);
					}else{
						sendDone(who, id);
					}
				} break;												
			}
		}
		Vector<String> timeoutQueue;
		int amnt = 0;
		pthread_mutex_lock(&ftSessionMutex[1]);
		for(auto &it : client->sessions){
			++amnt;
			auto &se = it.second;
			if(se.getLastPing() > 1000 && se.getLastPing() < 60*1000 && nite::getTicks()-se.lastResend > 1000){
				// request never reached server. try again
				if(!se.sender && se.index == 0){
					sendRequest(se.ip, se.id, se.indexed.hash);
					se.lastResend = nite::getTicks();
				}else
				// chunk never arrived, send ok to request it again
				if(!se.sender && se.index > 0){
					sendOK(se.ip, se.id, se.index);
					se.lastResend = nite::getTicks();
				}				
				// server is done, send done to client just in case
				if(se.sender && se.index == se.indexed.size){
					sendDone(se.ip, se.id);
					se.lastResend = nite::getTicks();
				}
			}
			// timeout, kill it
			if(se.getLastPing() > 60*1000){
				sendCancel(se.ip, se.indexed.hash, "timeout");
				timeoutQueue.push_back(se.id);
			}
		}
		pthread_mutex_unlock(&ftSessionMutex[1]);
		for(int i = 0; i < timeoutQueue.size(); ++i){
			killFTSession(timeoutQueue[i], client, "timeout");
		}
		if(amnt == 0){
			nite::sleep(16);
		}
	}
    return NULL; 
} 
static Vector<__FTListen> listenings;
static void killListener(nite::FileTransfer::UDPClient *client){
	pthread_mutex_lock(&ftListeningMutex[0]); 
	for(int i = 0; i < listenings.size(); ++i){
		if(listenings[i].client == client){
			nite::print("killed FT listening session at "+nite::toStr(client->sock.port));
			pthread_cancel(listenings[i].thread);
			listenings.erase(listenings.begin() + i);
			break;
		}
	}
	pthread_mutex_unlock(&ftListeningMutex[0]);
}
static void spawnListener(nite::FileTransfer::UDPClient *client){
	pthread_mutex_lock(&ftListeningMutex[0]); 
	__FTListen ns;
	ns.client = client;
	pthread_create(&ns.thread, NULL, __FTListeningThread, client); 
	listenings.emplace_back(ns);
	nite::print("spawned FT listening session at "+nite::toStr(client->port));
	pthread_mutex_unlock(&ftListeningMutex[0]);
}

void nite::FileTransfer::end(){
	Vector<nite::FileTransfer::UDPClient*> listenersQueue;
	for(auto &it : listenings){
		listenersQueue.push_back(it.client);
	}
	for(int i = 0; i < listenersQueue.size(); ++i){
		killListener(listenersQueue[i]);
	}	
}

nite::FileTransfer::UDPClient::UDPClient(){
	listening = false;
	port = nite::NetworkDefaultFileTransferPort;
}

nite::FileTransfer::UDPClient::~UDPClient(){
	clear();
}

nite::FileTransfer::IndexedFile *nite::FileTransfer::UDPClient::indexFile(const String &file){
	if(!nite::fileExists(file)){
		nite::print("failed to index file '"+file+"': it doesn't exist");
		return NULL;
	}
	String hash = nite::hashFile(file);
	if(indexed.find(hash) != indexed.end()){
		nite::print("file '"+file+"'('"+hash+"') is already indexed");
		return NULL;
	}
	IndexedFile indexed;
	indexed.path = file;
	indexed.hash = hash;
	indexed.size = nite::fileSize(file);
	this->indexed[hash] = indexed;
	return &this->indexed[hash];
}

void nite::FileTransfer::UDPClient::indexDir(const String &path){
	Vector<String> files;
	nite::fileFind(path, nite::Find::File, "", false, false, files);
	int n = 0;
	for(int i = 0; i < files.size(); ++i){
		String rp = path + "/" + files[i];
		IndexedFile file;
		file.path = rp;
		file.hash = nite::hashFile(rp);
		file.size = nite::fileSize(rp);
		this->indexed[file.hash] = file;
		++n;
	}
	nite::print("indexed '"+path+"': found "+nite::toStr(n)+" file(s)");
}

void nite::FileTransfer::UDPClient::listen(UInt16 port){
	if(listening){
		nite::print("FT client is already listening at "+nite::toStr(this->port)+": stop it first");
		return;
	}
	this->port = port;
	listening = true;
	spawnListener(this);
}

void nite::FileTransfer::UDPClient::clear(){
	stop("");
	if(listening){
		killListener(this);
	}
	listening = false;
}

void nite::FileTransfer::UDPClient::stop(const String &id){
	if(id == ""){
		Vector<String> toDelete;
		pthread_mutex_lock(&ftSessionMutex[1]);
		for(auto it : sessions){
			toDelete.push_back(it.second.id);
		}
		pthread_mutex_unlock(&ftSessionMutex[1]);
		for(int i = 0; i < toDelete.size(); ++i){
			killFTSession(toDelete[i], this, "cancelled by client");
		}
	}else{
		killFTSession(id, this, "cancelled by client");
	}
}

void nite::FileTransfer::UDPClient::request(	const nite::IP_Port &client,
												const String &hash, 
												const String &output, 
												const nite::FileTransfer::FTCallback &callback,
												bool overwrite){
	if(!sock.opened){
		nite::print("failed to request file '"+hash+"' from server: socket is not open. run listen() first");
		return;
	}
	if(nite::fileExists(output)){
		if(overwrite){
			nite::removeFile(output);
		}else{
			nite::print("cannot request FT to output to '"+output+"': it already exists and overwritting is not allowed");
			return;
		}
	}
	auto file = FileTransfer::IndexedFile();
	file.path = output;
	file.hash = hash;
	file.size = 0;
	indexed[hash] = file;
	nite::Packet request;
	request.setHeader(FTHeader::REQUEST);
	auto id = initFTSession(file, client, false, this, callback);
	request.write(hash);
	request.write(id);
	sock.send(client, request);
}