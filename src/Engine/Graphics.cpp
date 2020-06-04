#define NO_SDL_GLEXT
#include <fstream>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "GLEW/GL/glew.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "Network.hpp"
#include "UI/UI.hpp"
#include "Tools/Tools.hpp"
#include "Input.hpp"
#include "Shader.hpp"
#include "Graphics.hpp"
#include "View.hpp"
#include "Console.hpp"
#include "../Game.hpp"

using namespace nite;

static Vec2 size = Vec2(1280, 720);
static nite::Color currentColor = nite::White;
static int depth = 0;
static int target = RenderTargetGame;
static int bitsPerPixel	= 32;
static bool autoResolution = false;
static bool enableFullscreen = false;
static bool	enableVSync	= true;
static bool	enableBorderless	= false;
static bool	enableResizable	= false;
static bool graphicsInitd = false;
static SDL_Window *Window;
static SDL_GLContext Context;
static float targetExcess = 1.0f;
void flushTexture();
void flushFont();

static void readConfigFile(){
	nite::print("reading data/config.json file");
	Jzon::Parser parser;
	Jzon::Node node = parser.parseFile("data/config.json");
	size = Vec2(node.get("width").toInt(1280), node.get("height").toInt(720));
	bitsPerPixel	= node.get("bpp").toInt(32);
	autoResolution = node.get("autoresolution").toBool(false);
	enableFullscreen = node.get("fullscreen").toBool(false);
	enableVSync	= node.get("vsync").toBool(true);
	enableBorderless	= node.get("borderless").toBool(false);
	enableResizable	= node.get("resizeable").toBool(false);
	targetExcess = node.get("targetexcess").toFloat(1.0f);
}

static nite::Console::Result writeConfigFile(){
	Jzon::Node node = Jzon::object();
	node.add("width", (int)size.x);
	node.add("height", (int)size.y);
	node.add("bpp", bitsPerPixel);
	node.add("fullscreen", enableFullscreen);
	node.add("autoresolution", autoResolution);
	node.add("vsync", enableVSync);
	node.add("borderless", enableBorderless);
	node.add("resizeable", enableResizable);
	node.add("targetexcess", targetExcess);
	Jzon::Writer writer;
	std::ofstream file("data/config.json", std::ios_base::out);
	writer.writeStream(node, file);
	return nite::Console::Result("wrote data/config.json", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

static nite::Console::Result cfWriteconfig(Vector<String> params){
	return writeConfigFile();
}
static auto cfWriteConfigIns = nite::Console::CreateFunction("write_config", &cfWriteconfig);

static void cfWriteconfigChanged(Vector<String> params){
  writeConfigFile();
	nite::print("restart to see changes. type command 'restart'");
}
static nite::Console::CreateProxy cpRenWidth("ren_width", nite::Console::ProxyType::Float, sizeof(float), &size.x, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenHeight("ren_height", nite::Console::ProxyType::Float, sizeof(float), &size.y, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenBpp("ren_bpp", nite::Console::ProxyType::Int, sizeof(int), &bitsPerPixel, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenAutoReso("ren_autoresolution", nite::Console::ProxyType::Bool, sizeof(bool), &autoResolution, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenFs("ren_fullscreen", nite::Console::ProxyType::Bool, sizeof(bool), &enableFullscreen, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenVsync("ren_vsync", nite::Console::ProxyType::Bool, sizeof(bool), &enableVSync, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenBorderless("ren_borderless", nite::Console::ProxyType::Bool, sizeof(bool), &enableBorderless, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenResizeable("ren_resizeable", nite::Console::ProxyType::Bool, sizeof(bool), &enableResizable, &cfWriteconfigChanged);
static nite::Console::CreateProxy cpRenTargetExcess("ren_targetexcess", nite::Console::ProxyType::Float, sizeof(float), &targetExcess, &cfWriteconfigChanged);

static void setupTarget(){
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,0);
	glViewport(0, 0, size.x * targetExcess, size.y * targetExcess);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, size.x * targetExcess, size.y * targetExcess, 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void setupTarget(float x, float y){
	glClearColor(0, 0, 0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, x, 0, y, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static String getOpenGLError(int v){
	switch(v){
		case 0x0500:
			return "GL_INVALID_ENUM";
		case 0x0501:
			return "GL_INVALID_VALUE";
		case 0x0502:
			return "GL_INVALID_OPERATION";
		case 0x0503:
			return "GL_STACK_OVERFLOW";
		case 0x0504:
			return "GL_STACK_UNDERFLOW";
		case 0x0505:
			return "GL_OUT_OF_MEMORY";
		case 0x0506:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case 0x0507:
			return "GL_CONTEXT_LOST";
		case 0x8031:
			return "GL_TABLE_TOO_LARGE1";
		default:
			return "UNDEFINED";
	}
}

static String handleOpenGLFrameBufferError(){
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE:
		    return "[FB OK]";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		    return "An attachment could not be bound to frame buffer object!";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		    return "Attachments are missing! At least one image (texture) must be bound to the frame buffer object!";
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		    return "The dimensions of the buffers attached to the currently used frame buffer object do not match!";
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		    return "The formats of the currently used frame buffer object are not supported or do not fit together!";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		    return "A Draw buffer is incomplete or undefinied. All draw buffers must specify attachment points that have images attached.";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		    return "A Read buffer is incomplete or undefinied. All read buffers must specify attachment points that have images attached.";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		    return "All images must have the same number of multisample samples.";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS :
		    return "If a layered image is attached to one attachment, then all attachments must be layered attachments. The attached layers do not have to have the same number of layers, nor do the layers have to come from the same kind of texture.";
		case GL_FRAMEBUFFER_UNSUPPORTED:
		    return "Attempt to use an unsupported format combinaton!";
		default:
		    return "Unknown error while attempting to create frame buffer object!";
	}
}

static void handleOpenGLError(const String &at){
	static GLenum err;
	String fbError = handleOpenGLFrameBufferError();
	while ((err = glGetError()) != GL_NO_ERROR){
			nite::print("OpenGL"+(at.length() > 0 ? " '"+at+"' " : "")+": "+getOpenGLError(err)+" "+fbError);
	}
}

static void handleOpenGLError(){
	handleOpenGLError("");
}

nite::Uniform::~Uniform(){
	shaderName.clear();
	floats.clear();
	integers.clear();
	vectors.clear();
}

void nite::Uniform::clear(){
	shaderName.clear();
	floats.clear();
	integers.clear();
	vectors.clear();
}

nite::Program::~Program(){
	shaderName.clear();
}

void nite::setColor(const nite::Color &color){
	currentColor = color;
}

void nite::setColor(float r, float g, float b, float a){
	currentColor = nite::Color(r, g, b, a);
}

void nite::resetColor(){
	currentColor.r 	= 1.0f;
	currentColor.g 	= 1.0f;
	currentColor.b 	= 1.0f;
	currentColor.a 	= 1.0f;
}

void nite::setAlpha(float a){
	currentColor.a = nite::round(a);
}

nite::Color &nite::getColor(){
	return currentColor;
}

unsigned nite::getWidth(){
	return size.x;
}

unsigned nite::getHeight(){
	return size.y;
}

Vec2 nite::getSize(){
	return size;
}

void nite::setDepth(int D){
	depth = D;
}
int nite::getDepth(){
	return depth;
}

unsigned nite::getRenderTargetTextureId(unsigned T){

}

void nite::setRenderTarget(unsigned T){
	target = T;
}

unsigned nite::getRenderTarget(){
	return target;
}

nite::Renderable::~Renderable(){
	for(int i = 0; i < programs.size(); ++i){
		//programs[i]->~Program();
		delete programs[i];
	}
	programs.clear();
}

void nite::Renderable::apply(nite::Shader &shader){
	nite::Program *program = new nite::Program(shader.getProgram(), shader.fragFilename, &shader);
	this->programs.push_back(program);
}

void nite::Renderable::apply(nite::Shader &shader, Uniform uniforms){
	nite::Program *program = new nite::Program(shader.getProgram(), uniforms, shader.fragFilename, &shader);
	uniforms.shaderName = shader.fragFilename;
	this->programs.push_back(program);
}

static GLuint createTexture(int Width, int Height){
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA8,
			Width,
			Height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			0
	);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

struct RenderTarget {
	Vector<Renderable*> objects;
	GLuint objectId;
	GLuint texture;
	float Zoom;
	float realZoom;
	bool smooth;
	float realAngle;
	float angle;
	float minZoom;
	Vector<Program> programs;
	void init(){
		/* Create framebuffer for the scene */
		glGenFramebuffers(1, &objectId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, objectId);
		/* Create texture and attach it to the framebuffer */
		texture = createTexture(size.x * targetExcess, size.y * targetExcess);
		/* Attach it to the freambuffer */
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		/* Get status */
		// GLenum Status;
		// Status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		// if (Status != GL_FRAMEBUFFER_COMPLETE_EXT) Error = Status;

		/* Switch back to the window framebuffer */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		minZoom = 25;
		angle 	= 0;
		Zoom 		= 100;
		realZoom= Zoom;
		target	= 0;
		smooth 	= true;
	}
	RenderTarget(){
		clear();
		objects.reserve(32768);
	}
	void setZoom(float Z){
		Zoom = Z*100.0f;
		if(Zoom < minZoom) Zoom = minZoom;
	}
	float setZoom(float Z, float mu){
		nite::lerpDiscrete(realZoom, Z * 100.0f, mu);
		if(realZoom < minZoom){
			realZoom = minZoom;
		}
		Zoom = realZoom;
	}
	void setAngle(float A){
		angle = A;
	}
	void clear(){
		for(unsigned i=0; i<objects.size(); i++){
			//objects[i]->~Renderable();
			delete objects[i];
		}
		programs.clear();
		objects.clear();
	}
	void add(Renderable *object){
		object->depth = depth;
		for(unsigned i = 0; i < objects.size(); ++i){
			if(objects[i]->depth < depth){
				objects.insert(objects.begin()+i, object);
				return;
			}
		}
		for(int i = objects.size()-1; i >= 0; i--){
			if(objects[i]->depth == depth){
				objects.insert(objects.begin()+i+1, object);
				return;
			}
		}
		objects.insert(objects.end(), object);

	}
};

static RenderTarget targets[RenderTargetNumber];

void nite::setZoom(unsigned T, float Z, float mu){
    targets[T].setZoom(Z, mu);	
}

void nite::setZoom(unsigned T, float Z){
    targets[T].setZoom(Z);
}

void nite::setAngle(unsigned T, float Z){
    targets[T].setAngle(Z);
}

void nite::setZoomDiscrete(unsigned T, float Z){
	targets[T].Zoom = Z * 100.0f;
	targets[T].realZoom = Z * 100.0f;
}

namespace nite {
	void addProgram(unsigned T, GLuint ProgramId, const String &name, nite::Shader *ref){
        targets[T].programs.push_back(Program(ProgramId, name, ref));
	}
	void addProgram(unsigned T, GLuint ProgramId, Uniform uniforms, const String &name, nite::Shader *ref){
        targets[T].programs.push_back(Program(ProgramId, uniforms, name, ref));
	}
}

static nite::Batch *currentBatch = NULL;

static inline bool isClippingOut(float x, float y, float w, float h, float origX, float origY){
	nite::Vec2 size = nite::getSize() * 2.0f;
	nite::Vec2 v = nite::getView(nite::getRenderTarget()) - size / 4.0f;
	nite::Vec2 origCalc = nite::Vec2(w*origX, h*origY);
	nite::Vec2 p = nite::Vec2(x, y);
	nite::Vec2 s = nite::Vec2(w, h) - origCalc;
	return 	(p.x - s.x < v.x && p.x + s.x < v.x) || (p.x + s.x > v.x + size.x && p.x - s.x > v.x + size.x) ||
					(p.y - s.y < v.y && p.y + s.y < v.y) || (p.y + s.y > v.y + size.y && p.y - s.y > v.y + size.y);
	//return false;
}

struct BatchT {
	bool empty;
	GLuint textureId;
	GLuint framebufferId;
	String hash;
	Vector<nite::Batch*> owners;
	BatchT(){
		empty = true;
	}
	void clear(){
		if(empty) return;
		empty = true;
		hash = "NULL";
		glDeleteTextures(1, &textureId);
		glDeleteFramebuffers(1, &framebufferId);
	}
};

static Vector<BatchT> batches;

static int getEmptySlot(){
	for(int i=0; i<batches.size(); ++i){
		if(batches[i].empty){
			return i;
		}
	}
	int n = batches.size();
	batches.push_back(BatchT());
	return n;
}

static int getSlotByHash(const String &Hash){
	int d = -1;
	for(int i=0; i<batches.size(); ++i){
		if(batches[i].empty) continue;
		if(batches[i].hash == Hash){
			d = i;
			break;
		}
	}
	return d;
}

static bool renTextureBatches = true;
static nite::Console::CreateProxy cpRenBatches("ren_batches", nite::Console::ProxyType::Bool, sizeof(bool), &renTextureBatches);

static void drawBatch(nite::Renderable *object){
	if(!renTextureBatches) return;
	flushTexture();
	flushFont();
	nite::RenderableBatch &obj = *(nite::RenderableBatch*)object;
	GLuint currentBind = batches[obj.objectId].textureId;
	glColor4f(obj.color.r, obj.color.g, obj.color.b, obj.color.a);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	nite::Vec2 offset = obj.target == nite::RenderTargetDummy ? nite::Vec2(0.0f) : nite::getRenderOffset();
	glTranslatef(obj.position.x - nite::getViewX(obj.target) + offset.x, obj.position.y - nite::getViewY(obj.target) + offset.y, 0.f);
	if(obj.angle != 0) glRotatef(obj.angle, 0.0, 0.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, currentBind);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, obj.smooth ? GL_LINEAR : GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, obj.smooth ? GL_LINEAR : GL_NEAREST);
	for(int i = 0; i < obj.programs.size(); ++i){
		glUseProgram(obj.programs[i]->id);
		for(const auto& Uniform : obj.programs[i]->uniforms.colors){
			int tex = glGetUniformLocation(obj.programs[i]->id, Uniform.first.c_str());
			if(tex != -1){
				float v[3];
				v[0] = Uniform.second.r;
				v[1] = Uniform.second.g;
				v[2] = Uniform.second.b;
				glUniform3fv(tex, 1, v);
			}else{
				if(!obj.programs[i]->ref->faulty){
					nite::print("'"+obj.programs[i]->shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					obj.programs[i]->ref->faulty = true;
				}
			}
		}
		for(const auto& Uniform : obj.programs[i]->uniforms.integers){
			int tex = glGetUniformLocation(obj.programs[i]->id, Uniform.first.c_str());
			if(tex != -1){
				glUniform1i(tex, Uniform.second);
			}else{
				if(!obj.programs[i]->ref->faulty){
					nite::print("'"+obj.programs[i]->shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					obj.programs[i]->ref->faulty = true;
				}
			}
		}
		for(const auto& Uniform : obj.programs[i]->uniforms.floats){
			int tex = glGetUniformLocation(obj.programs[i]->id, Uniform.first.c_str());
			if(tex != -1){
				glUniform1f(tex, Uniform.second);
			}else{
				if(!obj.programs[i]->ref->faulty){
					nite::print("'"+obj.programs[i]->shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					obj.programs[i]->ref->faulty = true;
				}
			}
		}
		for(const auto& Uniform : obj.programs[i]->uniforms.vectors){
			int tex = glGetUniformLocation(obj.programs[i]->id, Uniform.first.c_str());
			if(tex != -1){
				float v[2];
				v[0] = Uniform.second.x;
				v[1] = Uniform.second.y;
				glUniform2fv(tex, 1, v);
			}else{
				if(!obj.programs[i]->ref->faulty){
					nite::print("'"+obj.programs[i]->shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					obj.programs[i]->ref->faulty = true;
				}
			}
		}
	}
	GLfloat Box[] = {
		-obj.origin.x, 									-obj.origin.y,
		obj.size.x*obj.scale.x-obj.origin.x, 			-obj.origin.y,
		obj.size.x*obj.scale.x-obj.origin.x, 			obj.size.y*obj.scale.y-obj.origin.y,
		-obj.origin.x,									obj.size.y*obj.scale.y-obj.origin.y,
	};
	GLfloat texBox[] = {
		obj.region.x / obj.origSize.x,						obj.region.y / obj.origSize.y,
		(obj.region.x + obj.region.w) / obj.origSize.x,	 	obj.region.y / obj.origSize.y,
		(obj.region.x + obj.region.w) / obj.origSize.x,		(obj.region.y + obj.region.h) / obj.origSize.y,
		obj.region.x / obj.origSize.x,						(obj.region.y + obj.region.h) / obj.origSize.y
	};
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, Box);
	glTexCoordPointer(2, GL_FLOAT, 0, texBox);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	for(int i = 0; i < obj.programs.size(); ++i){
		delete obj.programs[i];
	}
	obj.programs.clear();
}

void nite::addRenderList(Renderable *object){
	if(currentBatch != NULL){
		object->target = nite::RenderTargetDummy;
		currentBatch->add(object);
		return;
	}
	object->target = target;
	targets[target].add(object);
}

bool nite::isItOnBatch(){
	return currentBatch != NULL;
}

void nite::Batch::add(Renderable *object){
	if(objectId <= -1) return;
	object->depth = depth;
	for(unsigned i = 0; i < objects.size(); ++i){
		if(objects[i]->depth < depth){
			objects.insert(objects.begin()+i, object);
			return;
		}
	}
	for(int i = objects.size()-1; i >= 0; i--){
		if(objects[i]->depth == depth){
			objects.insert(objects.begin()+i+1, object);
			return;
		}
	}
	objects.insert(objects.end(), object);
}

void nite::Batch::begin(){
	if(objectId <= -1) return;
	currentBatch = this;
}

void nite::Batch::end(){
	if(objectId <= -1) return;
	currentBatch = NULL;
}

void nite::Batch::clearBuffer(){
	if(objectId <= -1) return;
	for(unsigned i=0; i<objects.size(); i++){
		//objects[i]->~Renderable();
		delete objects[i];
	}
	objects.clear();
}

// TODO: add proper resource management for batches
void nite::Batch::unload(){	
	clearBuffer();	
	if (objectId <= -1 || batches.size() == 0) return;
	for(unsigned i=0; i<batches[objectId].owners.size(); i++){
		if (batches[objectId].owners[i] == this){
			batches[objectId].owners.erase(batches[objectId].owners.begin()+i);
			break;
		}
	}
	if (batches[objectId].owners.size() == 0){
		batches[objectId].clear();
	}
  	objectId = -1;	
}

nite::Batch& nite::Batch::operator= (const Batch &other){
	if(other.objectId <= -1) return *this;
	unload();
	scale = other.scale;
	region = other.region;	
	smooth = other.smooth;
	size = other.size;
	objectId = other.objectId;
	batches[objectId].owners.push_back(this);
	return *this;
}

nite::Batch::~Batch(){
	unload();
}

static int batchIdSeed = nite::randomInt(250, 500);
static int getBatchId(){
	return ++batchIdSeed;
}

void nite::Batch::init(int w, int h){
	if(objectId >= 0){
		if(w == size.x && h == size.y){
			return;
		}
		unload();
	}
	objectId = getEmptySlot();
	batches[objectId].owners.push_back(this);
	batches[objectId].empty = false;
	size.set(w, h);
	region.set(0, 0, w, h);
	scale.set(1.0f, 1.0f);
	glGenFramebuffers(1, &batches[objectId].framebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, batches[objectId].framebufferId);
	batches[objectId].textureId = createTexture(w, h);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, batches[objectId].textureId, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

nite::Batch::Batch(int w, int h){
	objectId = -1;	
}

nite::Batch::Batch(){
	objectId = -1;
}

nite::Batch::Batch(float w, float h){
	objectId = -1;	
	init(w, h);
}

nite::Batch::Batch(const nite::Vec2 &size){
	init(size.x, size.y);
}

RenderableBatch *nite::Batch::draw(float x, float y){
	if(objectId <= -1) return NULL;
	float rw = size.x;
	float rh = size.y;
	if(region.w != size.x || region.h != size.y){
		rw = region.w;
		rh = region.h;
	}
	nite::RenderableBatch *obj = new nite::RenderableBatch();
	obj->objectId = objectId;
	obj->origSize = size;
	obj->orig = this;
	obj->size.x	= rw;
	obj->size.y = rh;
	obj->position.x = x;
	obj->position.y = y;
	obj->angle = 0;
	obj->smooth	= smooth;
	obj->origin	= nite::Vec2(0, 0);
	obj->function	= &drawBatch;
	obj->color = nite::getColor();
	obj->region	= region;
	obj->scale = scale;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

RenderableBatch *nite::Batch::draw(float x, float y, float angle){
	if(objectId <= -1) return NULL;
	float rw = size.x;
	float rh = size.y;
	if(region.w != size.x || region.h != size.y){
		rw = region.w;
		rh = region.h;
	}
	nite::RenderableBatch *obj = new nite::RenderableBatch();
	obj->objectId = objectId;
	obj->origSize = size;
	obj->size.x = rw;
	obj->size.y	= rh;
	obj->orig = this;
	obj->position.x = x;
	obj->position.y = y;
	obj->angle = angle;
	obj->smooth = smooth;
	obj->origin = nite::Vec2(0, 0);
	obj->function = &drawBatch;
	obj->color = nite::getColor();
	obj->region	= region;
	obj->scale = scale;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

RenderableBatch *nite::Batch::draw(const nite::Vec2 &P){
	if(objectId <= -1) return NULL;
	return draw(P.x, P.y);
}

RenderableBatch *nite::Batch::draw(const nite::Vec2 &P, float angle){
	if(objectId <= -1) return NULL;
	return draw(P.x, P.y, angle);
}

RenderableBatch *nite::Batch::draw(const nite::Vec2 &P, const nite::Vec2 &S, const nite::Vec2 &orig, float angle){
	if(objectId <= -1) return NULL;
	return draw(P.x, P.y, S.x, S.y, orig.x, orig.y, angle);
}

RenderableBatch *nite::Batch::draw(float x, float y, float w, float h, float origX, float origY, float angle){
	if(objectId <= -1) return NULL;
	nite::RenderableBatch *obj = new nite::RenderableBatch();
	obj->objectId = objectId;
	obj->origSize = size;
	obj->size.x = w;
	obj->size.y = h;
	obj->orig = this;
	obj->position.x = x;
	obj->position.y = y;
	obj->origin.x = w*origX*scale.x;
	obj->origin.y = h*origY*scale.y;
	obj->angle = angle;
	obj->smooth	= smooth;
	obj->function	= &drawBatch;
	obj->color = nite::getColor();
	obj->region = region;
	obj->scale = scale;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

void nite::Batch::flush(){
	if(objectId <= -1) return;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, batches[objectId].framebufferId);
	setupTarget(size.x, size.y);
	for(int i = 0; i < objects.size(); ++i){
		objects[i]->function(objects[i]);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	clearBuffer();
}

static void setupOpenGL(){
	glClearColor(0, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, size.x, size.y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, size.x, size.y, 0, 1, -1);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_GL_SetSwapInterval(enableVSync);
}

nite::Vec2 nite::getRenderOffset(){
	return nite::Vec2(size.x * targetExcess - size.x, size.y * targetExcess - size.y) * 0.5f;
}

static void drawTarget(RenderTarget &target){
	//Game::Instance *g = Game::Get();
	nite::lerp(target.realZoom, target.Zoom, 0.10f);
	nite::lerp(target.realAngle, target.angle, 0.15f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	float w  = size.x * targetExcess * (target.realZoom/100.0f);
	float h  = size.y * targetExcess * (target.realZoom/100.0f);
	float wi = size.x * (target.realZoom/100.0f);
	float hi = size.y * (target.realZoom/100.0f) - size.y * 0.5f;
	float xOffset = (size.x * targetExcess - size.x) / 2.0f;
	float yOffset = (size.y * targetExcess - size.y) / 2.0f;
	float transX = 0 - xOffset;
	float transY = (h * (1.0f - 1.0f / targetExcess)) - yOffset;
	glTranslatef(transX + size.x, size.y * targetExcess * 0.5f, 0.0f);
	glRotatef(target.realAngle, 0.0, 0.0, 1.0);
	for(int i = 0; i < target.programs.size(); ++i){
		glUseProgram(target.programs[i].id);
		// nite::print(nite::toStr(target.programs[i].uniforms.integers.size()));
		// nite::print(nite::toStr(target.programs[i].uniforms.floats.size()));
		// nite::print(nite::toStr(target.programs[i].uniforms.vectors.size()));
		// int tex = glGetUniformLocation(target.programs[i].id, "image");
		// glActiveTexture(GL_TEXTURE0);
		// glUniform1i(tex, target.texture);
		
		for(const auto& Uniform : target.programs[i].uniforms.colors){
			int tex = glGetUniformLocation(target.programs[i].id, Uniform.first.c_str());
			if(tex != -1){
				float v[3];
				v[0] = Uniform.second.r;
				v[1] = Uniform.second.g;
				v[2] = Uniform.second.b;
				glUniform3fv(tex, 1, v);
			}else{
				if(!target.programs[i].ref->faulty){
					nite::print("'"+target.programs[i].shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					target.programs[i].ref->faulty = true;
				}				
			}
		}

		for(const auto& Uniform : target.programs[i].uniforms.integers){
			int tex = glGetUniformLocation(target.programs[i].id, Uniform.first.c_str());
			if(tex != -1){
				glUniform1i(tex, Uniform.second);
			}else{
				if(!target.programs[i].ref->faulty){
					nite::print("'"+target.programs[i].shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					target.programs[i].ref->faulty = true;
				}	
			}
		}
		for(const auto& Uniform : target.programs[i].uniforms.floats){
			int tex = glGetUniformLocation(target.programs[i].id, Uniform.first.c_str());
			if(tex != -1){
				glUniform1f(tex, Uniform.second);
			}else{
				if(!target.programs[i].ref->faulty){
					nite::print("'"+target.programs[i].shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					target.programs[i].ref->faulty = true;
				}	
			}
		}
		for(const auto& Uniform : target.programs[i].uniforms.vectors){
			int tex = glGetUniformLocation(target.programs[i].id, Uniform.first.c_str());
			if(tex != -1){
				float v[2];
				v[0] = Uniform.second.x;
				v[1] = Uniform.second.y;
				glUniform2fv(tex, 1, v);
			}else{
				if(!target.programs[i].ref->faulty){
					nite::print("'"+target.programs[i].shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
					target.programs[i].ref->faulty = true;
				}	
			}
		}
	}
	glBindTexture(GL_TEXTURE_2D, target.texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, target.smooth ? GL_LINEAR : GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, target.smooth ? GL_LINEAR : GL_NEAREST);
	GLfloat Box[] = {
		-wi, -hi,
		w-wi, -hi,
		w-wi, h-hi,
		-wi, h-hi,
	};
	GLfloat texBox[] = {
		0,		1,
		1,	 	1,
		1,		0,
		0,		0
	};
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, Box);
	glTexCoordPointer(2, GL_FLOAT, 0, texBox);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
}

static UInt64 initDelta;
static UInt64 currentDelta;
static float globalTimescale = 1.0f;

static nite::Console::CreateProxy cpGlobalTimescale("global_timescale", nite::Console::ProxyType::Float, sizeof(float), &globalTimescale);

float nite::getTimescale(){
	return globalTimescale;
}

void nite::setTimescale(float ts){
	globalTimescale = ts;
}

UInt64 nite::getDelta(){
	return currentDelta;
}

void nite::dropFrame(){
	size_t total = 0;
  for(int c = 0; c < RenderTargetNumber; ++c){
		if(targets[c].objects.size() == 0) continue;
		total += targets[c].objects.size();
    targets[c].clear();
  }
	nite::print("drop frame request: "+nite::toStr(total)+" objects");
}

void nite::graphicsRender(){

  	currentDelta = nite::getTicks() - initDelta;
	initDelta = nite::getTicks();

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.77, 0.77, 0.77, 1.0);
	for(int c = 0; c < RenderTargetNumber; ++c){
		if(targets[c].objects.size() == 0) continue;
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets[c].objectId);
		setupTarget();
		for(int i = 0; i < targets[c].objects.size(); ++i){
			targets[c].objects[i]->function(targets[c].objects[i]);
		}
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor (1, 1, 1, 1);

	for(int c = 0; c < RenderTargetNumber; ++c){
		if(targets[c].objects.size() == 0) continue;
		drawTarget(targets[c]);
	}

	for(int c = 0; c < RenderTargetNumber; ++c){
		if(targets[c].objects.size() == 0) continue;
		targets[c].clear();
	}

	nite::setDepth(0);
	flushTexture();
	flushFont();
	SDL_GL_SwapWindow(Window);

	nite::Console::render();
	nite::UI::render();	
}

static void gameExit(){
	static auto *ins = Game::getGameCoreInstance();
	if(!ins->isRunning){
		return;
	}
	nite::AsyncTask::update();
	nite::AsyncTask::end();	
	ins->end();	
	ins->onEnd();	
}

void ctrlC(int s){
	 gameExit();
}

static nite::Console::Result cfExit(Vector<String> params){
   gameExit();
   return nite::Console::Result();
}

static auto cfExitIns = nite::Console::CreateFunction("exit", &cfExit);
static auto cfQuitIns = nite::Console::CreateFunction("quit", &cfExit);

static Vector<String> parameters;

static nite::Console::Result cfRestart(Vector<String> params){
	nite::print("restarting...");
	gameExit();
	// restart
	char *newargv[parameters.size()];
	for(int i = 0; i < parameters.size(); ++i){
		auto &str = parameters[i];
		newargv[i] = new char[str.length()+1];
		strcpy(newargv[i], str.c_str());
	}
	execve((char*)parameters[0].c_str(), newargv, NULL); // TODO: fix this
	for(int i = 0; i < parameters.size(); ++i){
		delete newargv[i];
	}
	return nite::Console::Result();
}

static auto cfRestartIns = nite::Console::CreateFunction("restart", &cfRestart);

static float generalScale = -1.0f;
float nite::getGeneralScale(){
	if(generalScale == -1.0f){
	  auto ns = nite::getSize();
	  float cs = (ns.x + ns.y) * 0.5f;
	  float bs = (1280.0f + 720.0f) * 0.5f;
	  generalScale = cs / bs;
	}
	return generalScale;
}

void nite::setParameters(Vector<String> &params){
	parameters = params;
}

const Vector<String> &nite::getParameters(){
	return parameters;
}

bool nite::isGraphicsInit(){
	return graphicsInitd;
}

void nite::graphicsInit(){
	initDelta = nite::getTicks();
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
		String err = SDL_GetError();
		nite::print("fatal error: SDL was unable to start: "+err);
		 gameExit();
	}
	nite::socketInit();
    auto indexer = nite::getIndexer();
    indexer->indexDir("./data/map/");
    indexer->indexDir("./data/tileset/");
    indexer->indexDir("./data/texture/tileset/");
	indexer->indexDir("./data/texture/overworld/");
	signal(SIGINT, ctrlC);

	int Flags = SDL_WINDOW_OPENGL;

	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	auto displayWidth = DM.w;
	auto displayHeight = DM.h;

	readConfigFile();

	for(int i = 0; i < parameters.size(); ++i){
		if(parameters[i] == "-wd" || parameters[i] == "--windowed"){
			enableFullscreen = true;
		}else		
		if(parameters[i] == "-ars" || parameters[i] == "--auto-resolution"){
			autoResolution = true;
		}else
		if(parameters[i] == "-fs" || parameters[i] == "--fullscreen"){
			enableFullscreen = true;
		}else
		if(parameters[i] == "-rs" || parameters[i] == "--resizeable"){
			enableResizable = true;
		}else
		if(parameters[i] == "-bdl" || parameters[i] == "--borderless"){
			enableBorderless = true;
		}else
		if((parameters[i] == "-w" || parameters[i] == "--width") && i < parameters.size()-1){
			size.x = nite::toInt(parameters[i+1]);
		}else
		if((parameters[i] == "-h" || parameters[i] == "--height") && i < parameters.size()-1){
			size.y = nite::toInt(parameters[i+1]);
		}else
		if((parameters[i] == "-vsync" || parameters[i] == "--vertical-synchronization")){
			enableVSync = true;
		}else
		if((parameters[i] == "-nvsync" || parameters[i] == "--no-vertical-synchronization")){
			enableVSync = false;
		}											
	}

	if(enableFullscreen){
		Flags |= SDL_WINDOW_FULLSCREEN;
	}
	if(enableBorderless){
		Flags |= SDL_WINDOW_BORDERLESS;
	}
	if(enableResizable){
		Flags |= SDL_WINDOW_RESIZABLE;
	}
	if(autoResolution){
		size.set(displayWidth, displayHeight);
	}

	const char *n = "RID";

	Window = SDL_CreateWindow(n, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, Flags);
	
	if(Window == NULL){
		nite::print("failed to create window | size "+size.str()+": "+String(SDL_GetError()));
		nite::exit();
	}
	
	Context = SDL_GL_CreateContext(Window);
	SDL_GL_MakeCurrent(Window, Context);

	if (glewInit() != GLEW_OK){
		nite::print("GLEW failed to initialize.");
		nite::exit();
	}

	graphicsInitd = true;

	setupOpenGL();

	for(int c = 0; c < RenderTargetNumber; ++c){
		targets[c].init();
	}

	nite::print("nite engine | OS "+niteCurrentPlatform);
	nite::print("graphics init "+size.str()+" ("+(enableFullscreen ? String("fullscreen") : String("windowed"))+")");

}

void nite::graphicsEnd(){
	nite::print("graphics end");
	nite::Console::end();
	nite::FileTransfer::end();
	nite::socketEnd();
	SDL_DestroyWindow(Window);
	SDL_GL_DeleteContext(Context);
}
