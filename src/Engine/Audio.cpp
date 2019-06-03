#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
//#include "RaptorAudio/RaptorAudio.h"
//#include "RaptorAudio/SharedPointer.h"
#include "Tools/Tools.hpp"
#include "Audio.hpp"

//using namespace Raptor::Audio;
//using namespace Raptor::Utility;
#define RATE 44100
static bool init = false;
static float globalVolume = 1.0f;

struct SoundT {
	Vector<nite::Audio*> owners;
	bool empty;
	String Hash;
	char* song;
	size_t size;
    char *buffer;
	void clear(){
		if(buffer != NULL){
			delete buffer;
			size = 0;
		}
		delete song;
		//song.~StreamingSoundObject();
		song = NULL;
		buffer = NULL;
		empty = true;
	}
	SoundT(){
		size = 0;
		buffer = NULL;
		song = NULL;
		empty = true;
	}
	~SoundT(){

	}
};

static Vector<SoundT> soundList;

static int getEmptySlot(){
	for(int i=0; i<soundList.size(); ++i){
		if(soundList[i].empty){
			return i;
		}
	}
	int n = soundList.size();
	soundList.push_back(SoundT());
	return n;
}

static int getSlotByHash(const String &Hash){
	for(int i=0; i<soundList.size(); ++i){
		if(soundList[i].empty) continue;
		if(soundList[i].Hash == Hash){
			return i;
		}
	}
	return -1;
}

nite::Audio::Audio(const String &filename){
	objectId = -1;
	load(filename, nite::AudioMode::Stream);
}

nite::Audio::Audio(const nite::Audio &sound){
	objectId = -1;
	if(sound.objectId <= -1) return;
	objectId 	= sound.objectId;
	soundList[objectId].owners.push_back(this);
}

nite::Audio::Audio(){
	objectId = -1;
}

nite::Audio::~Audio(){
	unload();
}

nite::Audio& nite::Audio::operator= (const nite::Audio &other){
	if(other.objectId <= -1) return *this;
	unload();
	objectId 	= other.objectId;
	soundList[objectId].owners.push_back(this);
	return *this;
}

void nite::Audio::load(const String &filename, unsigned mode){
	if(!init){
		//SoundMixer::InitializeMixer(RATE, 4410, SoundMixerProfiles::SOUND_MIXER_SPEAKERS);
		init = false;
	}
	if(!nite::fileExists(filename)){
		nite::print("Error: Couldn't load '"+filename+"': It doesn't exist.");
		return;
	}
	this->filename = String(filename);
	size_t size = 0;
	char *buffer = NULL;
	//StreamingSoundObject *song = NULL;
	char *song = NULL;
	if(mode == nite::AudioMode::Stream){
		song = NULL;//new StreamingSoundObject(this->filename.c_str());
	}else
	if(mode == nite::AudioMode::Memory){
		std::ifstream file (filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open()){
			size = file.tellg();
			buffer = new char [size];
			file.seekg (0, std::ios::beg);
			file.read (buffer, size);
			file.close();
		}else{
			nite::print("Error: Couldn't load '"+filename+"': It failed to open.");
			return;
		}
		song = NULL; //new StreamingSoundObject(buffer, AudioOrigins::AUDIO_ORIGIN_OPENMEMORY_POINT, size);
	}

	String fileHash = nite::hashFile(filename);
	objectId = getSlotByHash(fileHash);
	if (objectId <= -1){
		objectId = getEmptySlot();
		soundList[objectId].empty = false;
		soundList[objectId].owners.push_back(this);
		soundList[objectId].Hash = fileHash;
	}else{
		soundList[objectId].owners.push_back(this);
		return;
	}

	soundList[objectId].song = song;
	soundList[objectId].buffer = buffer;
	soundList[objectId].size = size;
	nite::print("loaded audio `"+filename+"`.");
}

nite::Audio::Audio(const String &filename, unsigned mode){
	objectId = -1;
	load(filename, mode);
}

void nite::Audio::load(const String &filename){
	load(filename, nite::AudioMode::Stream);
}

void nite::Audio::unload(){
	if (objectId <= -1) return;
	for(unsigned i=0; i<soundList[objectId].owners.size(); i++){
		if (soundList[objectId].owners[i] == this){
			soundList[objectId].owners.erase(soundList[objectId].owners.begin()+i);
			break;
		}
	}
	if (soundList[objectId].owners.size() == 0){
		soundList[objectId].clear();
	}
	objectId = -1;
}

static int indexQueue = 0;
//static Dict <int, SoundObjectProperties> soundQueue;
#include "Graphics.hpp"
int nite::Audio::play(bool loop, float vol, float pitch){
	if(objectId <= -1) return -1;
	//SharedProperties sprop = CreateSharedProperties();
	//sprop->sp_PitchShift = pitch * nite::getTimescale();
	//sprop->sp_Volume = vol * globalVolume;
	//SoundObjectProperties prop = SoundMixer::GetMixer()->CreateProperties(soundList[objectId].song, sprop);
	//prop->SetLooping(loop);
	//SoundMixer::GetMixer()->PlaySoundObject(prop);
	//soundQueue[indexQueue++] = prop;
}

int nite::Audio::play(bool loop){
	return play(loop, 1.0f, 1.0f);
}

void nite::Audio::stop(int id){
	//if (soundQueue.find(id) == soundQueue.end()) return;
	//SoundMixer::GetMixer()->Stop(soundQueue[id]);
}

void nite::Audio::pause(int id){
	//if (soundQueue.find(id) == soundQueue.end()) return;
	///SoundMixer::GetMixer()->Pause(soundQueue[id]);
	//soundQueue.erase(id);
}

void nite::Audio::resume(int id){
	//if (soundQueue.find(id) == soundQueue.end()) return;
	//SoundMixer::GetMixer()->Resume(soundQueue[id]);
	//soundQueue.erase(id);
}
