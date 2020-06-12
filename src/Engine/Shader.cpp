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

 namespace nite {
	void addProgram(unsigned T, GLuint Program, const String &name, nite::Shader *ref);
	void addProgram(unsigned T, GLuint Program, Uniform uniforms, const String &name, nite::Shader *ref);
}

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


int getUniqueId(){
	static int seed = nite::randomInt(25, 75);
	return ++seed;
}

static int lastId = 1;
struct __ShaderData {
	int id;
	GLuint program;
	String hash; // frag
	Vector<nite::Shader*> borrows;
	String frag;
	String vert;
	String shaderName;
	__ShaderData(){
		id = ++lastId;
		program = 0;
	}
	void clear(){
		glDeleteProgram(program);
	}
	bool compile(const String &shaderName, const String &fragSource, const String &vertSource){
		nite::print("compiling shader...");
		const char *fragSrc = fragSource.c_str();
		const char *vertSrc = vertSource.c_str();
		this->shaderName = shaderName;
		// create shaders
		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		GLint Result = GL_FALSE;
		int logLength = 5000;
		char *buffer = new char[5000];
		memset(buffer, 0, logLength);
		String str;
		// compile vertex
		glShaderSource(vertShader, 1, &vertSrc, NULL);
		glCompileShader(vertShader);
		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
		memset(buffer, 0, logLength);
		glGetShaderInfoLog(vertShader, logLength, NULL, buffer);
		str = String(buffer);
		if(str.length() > 0) {
			nite::print("shader compilation error '"+shaderName+"' [Vertex]: \n\n"+str+"\n\n");
		}
		str = "";
		// compile frag
		glShaderSource(fragShader, 1, &fragSrc, NULL);
		glCompileShader(fragShader);
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
		memset(buffer, 0, logLength);
		glGetShaderInfoLog(fragShader, logLength, NULL, buffer);
		str = String(buffer);
		if(str.length() > 0) {
			nite::print("shader compilation error '"+shaderName+"' [Fragment]: \n\n"+str+"\n\n");
		}
		str = "";
		// put together
		program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);
		glLinkProgram(program);
		// check status
		glGetProgramiv(program, GL_LINK_STATUS, &Result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		memset(buffer, 0, logLength);
		glGetProgramInfoLog(program, logLength, NULL, buffer);
		str = String(buffer);
		if(str.length() > 0) {
			nite::print("Shader Error '"+shaderName+"' [Program]: \n\n"+str+"\n\n");
		}
		str = "";
		// clean
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
		delete buffer;
		return true;
	}
};
static Dict<int, __ShaderData> shaders;
static __ShaderData *getShaderData(const String &hash){
	for(auto &shader : shaders){
		if(shader.second.hash == hash){
			return &shaders[shader.first];
		}
	}
	return NULL;
}

// this doesn't work again... jeez
// this is ass. im gonna have to rework it from scratch
static nite::Console::Result cfReloadShaders(Vector<String> params){
	Dict<int, void*> reloads;

	for(auto &it : shaders){
		auto rit = reloads.find(it.second.id);
		if(it.second.borrows.size() > 0){
			void *ref = it.second.borrows[0]; // shouldn't be null
			static_cast<nite::Shader*>(ref)->reload();
			reloads[it.second.id] = ref;
		}
	}

	return nite::Console::Result("true", nite::Color(0.40f, 0.40f, 0.40f, 1.0f));
}

static auto cfReloadShadersIns = nite::Console::CreateFunction("shader_reload", &cfReloadShaders);


static String loadSource(const String &path){
    String Src;
    if (!nite::fileExists(path)){
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


void nite::Shader::reload(){
	if(id == -1){
		return;
	}
	String fragSource = loadSource(fragFilename);
	String vertSource = loadSource(vertFilename);	
	if(fragSource == "NULL" || vertSource == "NULL"){
		nite::print(":^)");
		return;
	}
	shaders[id].clear(); // delete from gpu
	shaders[id].compile(shaderName, fragSource, vertSource); // compile again
}

nite::Shader::Shader(){
	id = -1;
	faulty = false;
}

nite::Shader::~Shader(){
	unload();
}

nite::Shader::Shader(const String &frag, const String &vert){
	id = -1;
	load(frag, vert);
}

void nite::Shader::applyRenderTarget(unsigned T){
	if(id == -1){
		return;
	}
	addProgram(T, shaders[id].program, this->vertFilename, this);
}

void nite::Shader::applyRenderTarget(unsigned T, Uniform uniforms){
	if(id == -1){
		return;
	}
	addProgram(T, shaders[id].program, uniforms, this->vertFilename, this);
}

int nite::Shader::getProgram(){
	if(id == -1){
		return 0;
	}
	return shaders[id].program;
}

void nite::Shader::load(const String &frag, const String &vert){
	
	unload();

	fragFilename = frag;
	vertFilename = vert;
	shaderName = nite::getFilename(fragFilename); // removes path
	String fragSource = loadSource(frag);
	String vertSource = loadSource(vert);
	String merged = fragSource + vertSource;
	String hash = hashString(merged); // this might be too complex. TODO: optimize :^)
	auto *data = getShaderData(hash);

	if (data == NULL){
		__ShaderData _data;
		id = _data.id;
		shaders[id] = _data;
		shaders[id].borrows.push_back(this);
		shaders[id].hash = hash;
		shaders[id].frag = fragFilename;
		shaders[id].vert = vertFilename;
		data = &shaders[id];
	}else{
		id = data->id;
		this->shaderName = data->shaderName;
		this->fragFilename = data->frag;
		this->vertFilename = data->vert;
		data->borrows.push_back(this);
		return;
	}

	data->compile(shaderName, fragSource, vertSource);
	
	print("loaded shader '"+shaderName+"'.");
}

void nite::Shader::unload(){
	if (id == -1){
		return;
	}

	auto &borrows = shaders[id].borrows;

	for(unsigned i=0; i< borrows.size(); i++){
		if (borrows[i] == this){
			borrows.erase(borrows.begin() + i);
			break;
		}
	}

	if (borrows.size() == 0){
		shaders.erase(id);
	}

	faulty = false;
	id = -1;
}

bool nite::Shader::isLoaded(){
	return id != -1;
}

nite::Shader& nite::Shader::operator=(const nite::Shader &other){
	if(other.id == -1){
		return *this;
	}
	unload();
	id = other.id;
	fragFilename = other.fragFilename;
	vertFilename = other.vertFilename;
	shaderName = other.shaderName;
	shaders[id].borrows.push_back(this);
	return *this;
}
