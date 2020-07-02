#include "Indexer.hpp"
#include <pthread.h> 

static nite::Indexer indexer;
static pthread_mutex_t mutex; 

void nite::inderxerInit(){
    if(pthread_mutex_init(&mutex, NULL) != 0){ 
        nite::print("failed to start mutex");
        nite::exit();
    }     
}
void nite::indexerEnd(){
    pthread_mutex_destroy(&mutex);
}

nite::IndexedFile::IndexedFile(){
    lastRead = nite::getTicks();
}

void nite::IndexedFile::refresh(const String &path){
    if(!nite::fileExists(path)){
        return;
    }
    this->path = path;
    this->hash = nite::hashFile(path);
    this->size = nite::fileSize(path);
    this->lastRead = nite::getTicks();
    if(path == "./data/texture/overworld//player_sheet.png"){
        nite::print(path+" "+hash);
    }
    autotag();
}

void nite::IndexedFile::refresh(){
    refresh(path);
}

void nite::IndexedFile::autotag(){
    tags.clear();
    if(!nite::fileExists(path)){
        return;
    }
    // format/file type
    // TODO: add audio files
    auto format = nite::getFormat(path);
    if(format == ".png" || format == ".jpg" || format == ".jpeg" || format == "bmp"){
        tags.push_back("image");
    }
    if(format == "mp3" || format == "wav" || format == "ogg" || format == "flac"){
        tags.push_back("sound");
    }
    if(format == ".json"){
        tags.push_back("json");
    }
    if(format == ".ns"){
        nite::print("script");
    }
    // based on route
    if(path.find("data/tileset/") != -1 && isIt("json")){
        tags.push_back("tilesource");
    }
    if(path.find("data/texture/tileset/") != -1 && isIt("image")){
        tags.push_back("tileset");
    }    
    if(path.find("data/texture/overworld/") != -1 && isIt("image")){
        tags.push_back("spritesheet");
    }        
    if(path.find("data/map/") != -1 && isIt("json")){
        tags.push_back("map");
    }   
    if(path.find("data/map/generated") != -1 && isIt("map")){
        tags.push_back("generated");
        tags.push_back("ring");
    }      
    if(path.find("data/bgm/") != -1 && isIt("sound")){
        tags.push_back("bgm");
    }    
    if(path.find("data/sfx/") != -1 && isIt("sound")){
        tags.push_back("sfx");
    }        
}

bool nite::IndexedFile::isIt(const String &tag){
    for(int i = 0; i < tags.size(); ++i){
        if(tags[i] == tag){
            return true;
        }
    }
    return false;
}

nite::Indexer::Indexer(){

}

bool nite::Indexer::indexDir(const String &path){
    pthread_mutex_lock(&mutex); 
	Vector<String> files;
	nite::fileFind(path, nite::Find::File, "", false, false, files);
	int n = 0;
	for(int i = 0; i < files.size(); ++i){
		String rp = path + "/" + files[i];
		nite::IndexedFile file;
        file.refresh(rp);
		indexed[file.hash] = file;
		++n;
	}
    pthread_mutex_unlock(&mutex); 
    return true;
}

nite::IndexedFile *nite::Indexer::indexFile(const String &file){
	if(!nite::fileExists(file)){
		return NULL;
	}
	String hash = nite::hashFile(file);
	if(indexed.find(hash) != indexed.end()){
		nite::print("file '"+file+"'('"+hash+"') is already indexed");
		return NULL;
	}
	IndexedFile indexed;
	indexed.refresh(file);
    pthread_mutex_lock(&mutex); 
	this->indexed[hash] = indexed;
    pthread_mutex_unlock(&mutex); 
	return &this->indexed[hash];
}

nite::IndexedFile *nite::Indexer::indexGhostFile(const String &path, const String &hash){
   	nite::IndexedFile indexed;
	indexed.path = path;
    indexed.hash = hash;
    indexed.size = 0;
    pthread_mutex_lock(&mutex); 
	this->indexed[hash] = indexed;
    pthread_mutex_unlock(&mutex); 
	return &this->indexed[hash]; 
}

nite::IndexedFile *nite::Indexer::get(const String &hash){
    pthread_mutex_lock(&mutex); 
    auto it = indexed.find(hash);
    if(it == indexed.end()){
        return NULL;
    }
    pthread_mutex_unlock(&mutex); 
    return &it->second;
}


bool nite::Indexer::removeByPath(const String &path){
    pthread_mutex_lock(&mutex); 
    for(auto &it : indexed){
        if(it.second.path == path){
            indexed.erase(it.first);
            pthread_mutex_unlock(&mutex); 
            return true;
        }
    }
    pthread_mutex_unlock(&mutex); 
    return false;
}

bool nite::Indexer::removeByHash(const String &hash){
    pthread_mutex_lock(&mutex); 
    auto it = indexed.find(hash);
    if(it == indexed.end()){
        return false;
    }
    indexed.erase(hash);
    pthread_mutex_unlock(&mutex); 
    return true;
}

nite::Indexer *nite::getIndexer(){
    return &indexer;
}