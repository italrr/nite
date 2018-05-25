
#include "SOIL/SOIL.h"
#include "View.hpp"
#include "Tools/Tools.hpp"
#include "Texture.hpp"
#define NO_SDL_GLEXT
#include "Shader.hpp"
#include "GLEW/GL/glew.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "Console.hpp"

static bool renTextureSingles = true;
static bool renTextureBatches = true;
static nite::Console::CreateProxy cpRenBatches("ren_texturebatches", nite::Console::ProxyType::Bool, sizeof(bool), &renTextureBatches);
static nite::Console::CreateProxy cpRenTextures("ren_textures", nite::Console::ProxyType::Bool, sizeof(bool), &renTextureSingles);


struct textureT {
	GLuint texture;
	bool empty;
	String Hash;
	int Width;
	int Height;
	int Channels;
	bool stick;
  String filename;
	Vector<nite::Texture*> owners;
	void clear(){
		if(texture == 0) return;
    nite::print("unloaded texture `"+filename+"`.");
		empty = true;
		Hash = "NULL";
    filename = "NULL";
		stick = false;
		glDeleteTextures(1, &texture);
    texture = 0;
	}
	textureT(){
    Hash = "NULL";
    filename = "NULL";
		texture = 0;
		empty = 1;
		stick = false;
	}
	~textureT(){

	}
};

static Vector<textureT> textureList;

void flushTextureBuffer(){
  for(int i = 0; i < textureList.size(); ++i){
    textureList[i].clear();
  }
  textureList.clear();
}

static int getEmptySlot(){
	for(int i=0; i<textureList.size(); ++i){
		if(textureList[i].empty){
			return i;
		}
	}
	int n = textureList.size();
	textureList.push_back(textureT());
	return n;
}

static int getSlotByHash(const String &Hash){
	int d = -1;
	for(int i=0; i<textureList.size(); ++i){
		if(textureList[i].empty) continue;
		if(textureList[i].Hash == Hash){
			d = i;
			break;
		}
	}
	return d;
}

void nite::Texture::setStick(bool v){
	if(!isLoaded()) return;
	textureList[objectId].stick	= v;
}

int nite::Texture::getTextureId(){
	return !isLoaded() ? 0 : textureList[objectId].texture;
}

bool nite::Texture::isLoaded(){
	return objectId >= 0;
}

void nite::Texture::load(const String &path){
	nite::Color c;
	c.a = -1.0;
	load(path, c);
}

void nite::Texture::load(const String &path, const nite::Color &transparency){
	if(!nite::fileExists(path)){
		nite::print("Error: Couldn't load '"+path+"': It doesn't exist.");
		return;
	}
	unload();
	String fileHash = nite::hashMemory((char*)path.c_str(), path.length());
	objectId = getSlotByHash(fileHash);
	if (objectId <= -1){
		objectId = getEmptySlot();
		textureList[objectId].empty = 0;
		textureList[objectId].owners.push_back(this);
		textureList[objectId].Hash = fileHash;
    textureList[objectId].filename = path;
	}else{
		textureList[objectId].owners.push_back(this);
		region.set(0, 0, textureList[objectId].Width, textureList[objectId].Height);
		return;
	}

	int Width;
	int Height;
	int Channels;
	GLubyte *Pixels = SOIL_load_image(path.c_str(), &Width, &Height, &Channels, SOIL_LOAD_AUTO);
	if (Pixels == NULL){
		nite::print("Error: Couldn't load '"+path+"': Unsupported format.");
		return;
	}
	if(transparency.a != -1.0){
		for(int i = 0; i < Width * Height * Channels; i += 4){
			const nite::Color &tr = transparency;
			#define uchar unsigned char
			uchar &r = Pixels[i];
			uchar &g = Pixels[i + 1];
			uchar &b = Pixels[i + 2];
			uchar &a = Pixels[i + 3];
			if(r == (tr.r * 255.0) && g == (tr.g * 255.0) && b == (tr.b * 255.0)){
				r = 0; g = 0;	b = 0; a = 0;
			}
		}
	}
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenTextures(1, &textureList[objectId].texture);
	glBindTexture(GL_TEXTURE_2D, textureList[objectId].texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	textureList[objectId].Width		= Width;
	textureList[objectId].Height		= Height;
	textureList[objectId].Channels	= Channels;
	region.set(0, 0, Width, Height);
	nite::print("loaded texture '"+path+"'.");
	SOIL_free_image_data(Pixels);
}

nite::Texture::Texture(){
	objectId = -1;
	smooth  = false;
	scale = nite::Vec2(1, 1);
}

nite::Texture::Texture(const String &path){
	objectId = -1;
	smooth = false;
	scale = nite::Vec2(1, 1);
	load(path);
}

nite::Texture::Texture(const String &path, const nite::Color &transparency){
	objectId = -1;
	smooth = false;
	scale = nite::Vec2(1, 1);
	load(path, transparency);
}

nite::Texture::Texture(const nite::Texture &other){
	if(other.objectId <= -1) return;
	scale = other.scale;
	region = other.region;
	smooth = other.smooth;
	objectId = other.objectId;
	textureList[objectId].owners.push_back(this);
}

void nite::Texture::unload(){
	if (!isLoaded() || textureList.size() == 0) return;
	for(unsigned i=0; i<textureList[objectId].owners.size(); i++){
		if (textureList[objectId].owners[i] == this){
			textureList[objectId].owners.erase(textureList[objectId].owners.begin()+i);
			break;
		}
	}
	if (!textureList[objectId].stick && textureList[objectId].owners.size() == 0){
		textureList[objectId].clear();
	}
  objectId = -1;
}

nite::Texture::~Texture(){
	unload();
}


static GLuint lastBind = 0;

void flushTexture(){
	lastBind = 0;
}

void flushFont();

static void drawTexture(nite::Renderable *object){
	if(!renTextureSingles) return;
	flushFont();
	nite::RenderableTextureT &obj = *(nite::RenderableTextureT*)object;
	GLuint currentBind = textureList[obj.objectId].texture;
	glColor4f(obj.color.r, obj.color.g, obj.color.b, obj.color.a);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	nite::Vec2 offset = obj.target == nite::RenderTargetDummy ? nite::Vec2(0.0f) : nite::getRenderOffset();
	glTranslatef(obj.position.x - nite::getViewX(obj.target) + offset.x, obj.position.y - nite::getViewY(obj.target) + offset.y, 0.f);
	if(obj.angle != 0) glRotatef(obj.angle, 0.0, 0.0, 1.0);
	if(currentBind != lastBind) // avoid re-binding the last texture.(an optimization, supposedly)
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
		obj.region.x / textureList[obj.objectId].Width,						obj.region.y / textureList[obj.objectId].Height,
		(obj.region.x + obj.region.w) / textureList[obj.objectId].Width,	 	obj.region.y / textureList[obj.objectId].Height,
		(obj.region.x + obj.region.w) / textureList[obj.objectId].Width,		(obj.region.y + obj.region.h) / textureList[obj.objectId].Height,
		obj.region.x / textureList[obj.objectId].Width,						(obj.region.y + obj.region.h) / textureList[obj.objectId].Height
	};
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, Box);
	glTexCoordPointer(2, GL_FLOAT, 0, texBox);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	for(int i = 0; i < obj.programs.size(); ++i){
		delete obj.programs[i];
	}
	lastBind = currentBind;
	obj.programs.clear();
}

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

static void drawTextureBatch(nite::Renderable *object){
	if(!renTextureBatches) return;
	flushFont();
	nite::RenderableTextureBatchT &obj = *(nite::RenderableTextureBatchT*)object;
	nite::TextureRegionBatch &batch = *(nite::TextureRegionBatch*)obj.batch;
	GLuint currentBind = textureList[obj.objectId].texture;
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	nite::Vec2 offset = obj.target == nite::RenderTargetDummy ? nite::Vec2(0.0f) : nite::getRenderOffset();
	float dxp = nite::round(batch.position.x - nite::getViewX(obj.target) + offset.x);
	float dyp = nite::round(batch.position.y - nite::getViewY(obj.target) + offset.y);
	glTranslatef(dxp, dyp, 0.0f);
	glRotatef(0.0f, 0.0, 0.0, 1.0);

	if(currentBind != lastBind) // avoid re-binding the last texture.(an optimization, supposedly)
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

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	for(int i = 0; i < batch.regions.size(); ++i){
		nite::TextureRegionSingle &si = batch.regions[i];
		if(isClippingOut(si.inDrawCoors.x + batch.position.x, si.inDrawCoors.y + batch.position.y, batch.size.x, batch.size.y, 0.0f, 0.0f)) continue;
		glVertexPointer(2, GL_FLOAT, 0, si.box);
		glTexCoordPointer(2, GL_FLOAT, 0, si.texBox);
		glDrawArrays(GL_QUADS, 0, 4);
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glUseProgram(0);
	for(int i = 0; i < obj.programs.size(); ++i){
		delete obj.programs[i];
	}
	lastBind = currentBind;
	obj.programs.clear();
}

nite::Texture& nite::Texture::operator= (const nite::Texture &other){
	if(other.objectId <= -1) return *this;
	unload();
	scale = other.scale;
	region = other.region;
	smooth = other.smooth;
	objectId = other.objectId;
	textureList[objectId].owners.push_back(this);
	return *this;
}

void nite::Texture::setSmooth(bool S){
	smooth = S;
}

nite::Vec2 nite::Texture::getSize(){
	return objectId <= -1 ? nite::Vec2() : nite::Vec2(textureList[objectId].Width, textureList[objectId].Height);
}

float nite::Texture::getWidth(){
	return objectId <= -1 ? 0 : textureList[objectId].Width;
}

float nite::Texture::getHeight(){
	return objectId <= -1 ? 0 : textureList[objectId].Height;
}

void nite::Texture::setRegion(float x, float y, float w, float h){
	region.set(x, y, w, h);
}

void nite::Texture::setRegion(const nite::Vec2 &P, const nite::Vec2 &S){
	region.set(P, S);
}

void nite::Texture::setRegion(const nite::Rect &R){
	region.set(R);
}

nite::RenderableTextureBatchT *nite::Texture::draw(nite::TextureRegionBatch *batch, float x, float y){
	if(objectId <= -1 || batch == NULL) return NULL;
	nite::RenderableTextureBatchT *obj = new nite::RenderableTextureBatchT();
	obj->objectId = objectId;
	obj->smooth	= smooth;
	obj->function	= &drawTextureBatch;
	obj->batch = batch;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableTextureT * nite::Texture::draw(float x, float y, float w, float h, float origX, float origY, float angle){
	if(objectId <= -1) return NULL;
	if(isClippingOut(x, y, w, h, origX, origY) && !isItOnBatch()) return NULL;
	nite::RenderableTextureT *obj = new nite::RenderableTextureT();
	obj->size.x			= w;
	obj->size.y 		= h;
	obj->position.x = x;
	obj->position.y = y;
	obj->objectId		= objectId;
	obj->origin.x 	= w*origX*scale.x;
	obj->origin.y 	= h*origY*scale.y;
	obj->angle 			= angle;
	obj->smooth 		= smooth;
	obj->function		= &drawTexture;
	obj->color			= nite::getColor();
	obj->region			= region;
	obj->scale			= scale;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableTextureT * nite::Texture::draw(float x, float y, float angle){
	if(objectId <= -1) return NULL;
	float rw = textureList[objectId].Width;
	float rh = textureList[objectId].Height;
	if(region.w != textureList[objectId].Width || region.h !=textureList[objectId].Height){
		rw = region.w;
		rh = region.h;
	}
	if(isClippingOut(x, y, rw, rh, 0.0f, 0.0f) && !isItOnBatch()) return NULL;
	nite::RenderableTextureT *obj = new nite::RenderableTextureT();
	obj->size.x			= rw;
	obj->size.y 		= rh;
	obj->position.x 	= x;
	obj->position.y 	= y;
	obj->objectId			= objectId;
	obj->angle 			= angle;
	obj->smooth 		= smooth;
	obj->origin			= nite::Vec2(0, 0);
	obj->function		= &drawTexture;
	obj->color			= nite::getColor();
	obj->region			= region;
	obj->scale			= scale;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableTextureT * nite::Texture::draw(float x, float y){
	if(objectId <= -1) return NULL;
	float rw = textureList[objectId].Width;
	float rh = textureList[objectId].Height;
	if(region.w != textureList[objectId].Width || region.h !=textureList[objectId].Height){
		rw = region.w;
		rh = region.h;
	}
	if(isClippingOut(x, y, rw, rh, 0.0f, 0.0f) && !isItOnBatch()) return NULL;
	nite::RenderableTextureT *obj = new nite::RenderableTextureT();
	obj->size.x			= rw;
	obj->size.y 		= rh;
	obj->position.x 	= x;
	obj->position.y 	= y;
	obj->objectId				= objectId;
	obj->angle 			= 0;
	obj->smooth 		= smooth;
	obj->origin			= nite::Vec2(0, 0);
	obj->function		= &drawTexture;
	obj->color			= nite::getColor();
	obj->region			= region;
	obj->scale			= scale;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableTextureT * nite::Texture::draw(const nite::Vec2 &P, const nite::Vec2 &S, const nite::Vec2 &orig, float angle){
	return draw(P.x, P.y, S.x, S.y, orig.x, orig.y, angle);
}

nite::RenderableTextureT * nite::Texture::draw(const nite::Vec2 &P, float angle){
	return draw(P.x, P.y, angle);
}

nite::RenderableTextureT * nite::Texture::draw(const nite::Vec2 &P){
	return draw(P.x, P.y);
}

void nite::Texture::setScale(float x, float y){
	scale.set(x, y);
}

void nite::Texture::setScale(const nite::Vec2 &S){
	scale.set(S);
}