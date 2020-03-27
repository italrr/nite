#define NO_SDL_GLEXT
#include "GLEW/GL/glew.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "Types.hpp"
#include "Tools/Tools.hpp"
#include "Shader.hpp"
#include "Console.hpp"
#include <fstream>
#include <string.h>

using namespace nite;

void nite::Uniform::add(String name, float n){
	floats[name] = n;
}

void nite::Uniform::add(String name, int n){
	integers[name] = n;
}

void nite::Uniform::add(String name, nite::Color c){
	colors[name] = c;
}

void nite::Uniform::add(String name, nite::Vec2 v){
	vectors[name] = v;
}

void nite::Uniform::remove(String name){
	integers.erase(name);
	floats.erase(name);
	vectors.erase(name);
}

struct ShaderT {
	GLuint Program;
	String Hash;
	Vector<void*> owners;
	String filename;
	bool empty;
	ShaderT(){
		empty = true;
	}
	void clear(){
		empty = true;
		// nite::print("unloaded shader '"+filename+"'");
		glDeleteProgram(Program);
	}
};

static Vector <ShaderT> Shaders;

static nite::Console::Result cfReloadShaders(Vector<String> params){
	Vector<nite::Shader*> shaders;
	for(int i = 0; i < Shaders.size(); ++i){
		for(int c = 0; c < Shaders[i].owners.size(); ++c){
			shaders.push_back((nite::Shader*)Shaders[i].owners[c]);
			(*((nite::Shader*)Shaders[i].owners[c])).unload();
		}
	}
	Shaders.clear();
	for(int i = 0; i < shaders.size(); ++i){
		nite::Shader &shader = (*((nite::Shader*)shaders[i]));
		shader.load(shader.getFragFilename(), shader.getVertFilename());
	}
	return nite::Console::Result();
}

static auto cfReloadShadersIns = nite::Console::CreateFunction("shader_reload", &cfReloadShaders);

static int getEmptySlot(){
	for(int i=0; i<Shaders.size(); ++i){
		if(Shaders[i].empty){
			return i;
		}
	}
	int n = Shaders.size();
	Shaders.push_back(ShaderT());
	return n;
}

static int findByHash(const String &Hash){
	for(int i = 0; i < Shaders.size(); ++i){
		if(Shaders[i].empty) continue;
		if(Shaders[i].Hash == Hash) return i;
	}
	return -1;
}

static String loadSource(const String &path){
    String Src;
    if (!fileExists(path)){
        return "NULL";
	}
    std::ifstream File;
    String Buffer;
    File.open(path.c_str());
    while (getline(File,Buffer)){
        Src += Buffer+"\n";
    }
    return Src;
}

String nite::Shader::getFragFilename(){
	return fragFilename;
}

String nite::Shader::getVertFilename(){
	return vertFilename;
}

void nite::Shader::reload(){
	if(objectId == -1) return;
	unload();
	load(fragFilename, vertFilename);
}

nite::Shader::Shader(){
	objectId = -1;
	faulty = false;
}

nite::Shader::~Shader(){
	unload();
}

nite::Shader::Shader(const String &frag, const String &vert){
	objectId = -1;
	load(frag, vert);
}

namespace nite {
	void addProgram(unsigned T, GLuint Program, const String &name, nite::Shader *ref);
	void addProgram(unsigned T, GLuint Program, Uniform uniforms, const String &name, nite::Shader *ref);
}

int nite::Shader::getProgram(){
	if(objectId == -1) return 0;
	return Shaders[objectId].Program;
}

void nite::Shader::applyRenderTarget(unsigned T){
	if(objectId == -1) return;
	addProgram(T, Shaders[objectId].Program, this->name, this);
}

void nite::Shader::applyRenderTarget(unsigned T, Uniform uniforms){
	if(objectId == -1) return;
	addProgram(T, Shaders[objectId].Program, uniforms, this->name, this);
}

String nite::Shader::getName(){
	return name;
}

void nite::Shader::load(const String &frag, const String &vert){
	fragFilename = frag;
	vertFilename = vert;
	String fragSource = loadSource(frag);
	String vertSource = loadSource(vert);
	String Merged = fragSource+vertSource;
	String shaderName = nite::formatRemove(nite::getFilename(frag));
	String Hash = hashMemory((char*)shaderName.c_str(), shaderName.length());
	objectId = findByHash(Hash);
	

	unload();

	if (objectId <= -1){
		objectId = getEmptySlot();
		Shaders[objectId].empty = false;
		Shaders[objectId].owners.push_back(this);
		Shaders[objectId].Hash = Hash;
    Shaders[objectId].filename = shaderName;
	}else{
		Shaders[objectId].owners.push_back(this);
		return;
	}

	const char *fragSrc = fragSource.c_str();
	const char *vertSrc = vertSource.c_str();

	/* Create shaders */
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int logLength = 5000;
	char *Buffer = new char[5000];
    memset(Buffer, 0, logLength);
	String str;


	/* Compile vertex */
	glShaderSource(vertShader, 1, &vertSrc, NULL);
	glCompileShader(vertShader);

	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
  	memset(Buffer, 0, logLength);
	glGetShaderInfoLog(vertShader, logLength, NULL, Buffer);
	str = String(Buffer);
	if(str.length() > 0) {
		nite::print("Shader Error '"+shaderName+"' [Vertex]: \n\n"+str+"\n\n");
	}
	str = "";

	glShaderSource(fragShader, 1, &fragSrc, NULL);
	glCompileShader(fragShader);

	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
  	memset(Buffer, 0, logLength);
	glGetShaderInfoLog(fragShader, logLength, NULL,Buffer);
	str = String(Buffer);
	if(str.length() > 0) {
		nite::print("Shader Error '"+shaderName+"' [Fragment]: \n\n"+str+"\n\n");
	}
	str = "";

	GLuint Program = glCreateProgram();
	glAttachShader(Program, vertShader);
	glAttachShader(Program, fragShader);
	glLinkProgram(Program);

	/* Check status */
	glGetProgramiv(Program, GL_LINK_STATUS, &Result);
	glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &logLength);
    memset(Buffer, 0, logLength);
	glGetProgramInfoLog(Program, logLength, NULL, Buffer);
	str = String(Buffer);
	if(str.length() > 0) {
		nite::print("Shader Error '"+shaderName+"' [Program]: \n\n"+str+"\n\n");
	}
	str = "";

	/* Clean up */
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	ShaderT New;
	this->name = shaderName;
	New.Program = Program;
	New.filename = shaderName;
	New.owners.push_back(this);
	New.Hash = Hash;
	Shaders.push_back(New);
	objectId = Shaders.size() - 1;
	delete Buffer;
	print("loaded shader '"+shaderName+"'.");
}

void nite::Shader::unload(){
	if (objectId == -1) return;
	for(unsigned i=0; i<Shaders[objectId].owners.size(); i++){
		if (Shaders[objectId].owners[i] == this){
			Shaders[objectId].owners.erase(Shaders[objectId].owners.begin()+i);
			break;
		}
	}
	if (Shaders[objectId].owners.size() == 0){
		Shaders[objectId].clear();
	}
	faulty = false;
	objectId = -1;
}

bool nite::Shader::isLoaded(){
	return objectId != -1;
}

nite::Shader& nite::Shader::operator= (const nite::Shader &other){
	if(other.objectId == -1) return *this;
	unload();
	objectId = other.objectId;
	Shaders[objectId].owners.push_back(this);
	return *this;
}
