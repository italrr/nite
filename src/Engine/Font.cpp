#include "FreeType/include/ft2build.h"
#include "FreeType/include/freetype/freetype.h"
#include "FreeType/include/freetype/ftglyph.h"
#include "FreeType/include/freetype/ftstroke.h"
#include FT_FREETYPE_H

#include  <algorithm>
#include "View.hpp"
#include "Tools/Tools.hpp"
#include "Font.hpp"
#include "Shader.hpp"

#define NO_SDL_GLEXT

#include "GLEW/GL/glew.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "SOIL/SOIL.h"
#include "Console.hpp"

static bool renText = true;

nite::Console::CreateProxy cpRenText("ren_text", nite::Console::ProxyType::Bool, sizeof(bool), &renText);

#define ASCII_MIN 32
#define ASCII_MAX 128
#define SCALING 1

#include <cmath>

struct glyphT {
	nite::Vec2 coors;
	nite::Vec2 size;
	nite::Vec2 orig;
	nite::Vec2 index;
};

struct fontT {
	glyphT glyphs[ASCII_MAX];
	GLuint atlas;
	String path;
	String hash;
	unsigned size;
	nite::Vec2 atlasSize;
	bool empty;
	float vertAdvance;
	Vector<nite::Font*> owners;
  	String filename;
	bool stick;
	fontT(){
		size = 0;
		empty = 1;
		hash = "NULL";
    	filename = "NULL";
		stick = false;
    	atlas = 0;
	}
	void clear(){
		if(atlas == 0) return;
		// nite::print("unloaded font `"+filename+"` ("+nite::toStr(size)+").");
		glDeleteTextures(1, &atlas);
    atlas = 0;
		hash = "NULL";
		empty = 1;
		stick = false;
	}
};

static Vector<fontT> fontList;

void flushFontBuffer(){
  for(int i = 0; i < fontList.size(); ++i){
    fontList[i].clear();
  }
  fontList.clear();
}

static unsigned getEmptySlot(){
	for(unsigned i=0; i<fontList.size(); ++i){
		if(fontList[i].empty){
			return i;
		}
	}
	unsigned i = fontList.size();
	fontList.push_back(fontT());
	return i;
}

static int getSlotByHash(const String &hash){
	for(unsigned i=0; i<fontList.size(); ++i){
		if(fontList[i].hash == hash){
			return i;
		}
	}
	return -1;
}


bool nite::Font::isLoaded(){
	return objectId >= 0;
}

void nite::Font::setTick(bool v){
	if(objectId <= -1) return;
	fontList[objectId].stick = v;
}

struct Single {
	int w, h;
	unsigned char *buffer;
	size_t size;
	Single(int w, int h, unsigned char *src){
		pure(w, h, src);
	}
	// pure makes a base with the exact same level per channel
	void pure(int w, int h, unsigned char* src){
		int channels = 4;
		this->w = w;
		this->h = h;
		size = w * h * channels;
		buffer = new unsigned char[size];
		memset(buffer, 0, size);
		for(int i = 0; i < w*h; ++i){
			buffer[i*4 + 0] = src[i];
			buffer[i*4 + 1] = src[i];
			buffer[i*4 + 2] = src[i];
			buffer[i*4 + 3] = src[i];
		}
	}
	// strong makes a base with strong opposite (> 0 -> 0, == 0 -> 255)
	void strong(int w, int h, unsigned char* src){
		int channels = 4;
		this->w = w;
		this->h = h;
		size = w * h * channels;
		buffer = new unsigned char[size];
		memset(buffer, 0, size);
		for(int i = 0; i < w*h; ++i){
			unsigned char v = src[i] > 0 ? 0 : 255;
			buffer[i*4 + 0] = v;
			buffer[i*4 + 1] = v;
			buffer[i*4 + 2] = v;
			buffer[i*4 + 3] = src[i];
		}
	}
	// bliz paints over a strong base
	void blitz(int w, int h, unsigned char* src){
		// blitz expects a smaller bitmap
		int offsetx = (this->w - w) / 2;
		int offsety = (this->h - h) / 2;
		for(int y = 0; y < h; ++y){
			for(int x = 0; x < w; ++x){
				int i = y * w  + x; //src
				int j = (y + offsety)  * this->w + x + offsetx; // target
				buffer[j*4 + 0] = src[i];
				buffer[j*4 + 1] = src[i];
				buffer[j*4 + 2] = src[i];
				// buffer[j*4 + 3] = (buffer[j*4 + 3] + src[i]); // disregard alpha			
			}
		}
	}
	Single(){
		buffer = NULL;
	}	
	void clear(){
		if(buffer != NULL){
			delete buffer;
		}
	}
};

// TODO: Unicode
void nite::Font::load(const String &path, unsigned size, float thickness){
	FT_Library library;
	FT_Face face;
	String filename = path;
	size *= SCALING;
	ln = -1.0f;

	if(!nite::fileExists(filename)){
		nite::print("couldn't load font '"+filename+"': doesn't exist");
		return;
	}

	if(FT_Init_FreeType(&library)){
		nite::print("failure starting FreeType: FT_Init_FreeType");
		return;
	}

	if(FT_New_Face(library, filename.c_str(), 0, &face)) {
		nite::print("failure loading font '"+filename+"': FT_New_Face");
		FT_Done_FreeType(library);
		return;
	}	

	unload();
	String pathAndSize = path+nite::toStr(size);
	String fileHash = nite::hashMemory((char*)pathAndSize.c_str(), pathAndSize.size());
	objectId = getSlotByHash(fileHash);
	if (objectId <= -1){
		objectId = getEmptySlot();
		fontList[objectId].empty = false;
		fontList[objectId].owners.push_back(this);
		fontList[objectId].hash = fileHash;
    	fontList[objectId].filename = path;
	}else{
		fontList[objectId].owners.push_back(this);
		return;
	}
	fontT &proxy = fontList[objectId];


	if(FT_Set_Char_Size(face, size << 6, size << 6, 96, 96)){
		nite::print("failure loading font '"+filename+"': FT_New_Face");
		FT_Done_FreeType(library);		
		return;
	}

	FT_Stroker stroker;
	if(thickness > 0.0f){
		FT_Stroker_New(library, &stroker);
		FT_Stroker_Set(stroker, (int)(thickness * 64.0f), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}
	Vector<Single> frags; // maybe a vector could be too slow?
	nite::Vec2 dims;
	int xcursor = 0.0f;
	// render characters
	for(int i = ASCII_MIN; i < ASCII_MAX; i++){
		FT_UInt cindex = FT_Get_Char_Index(face, i);
		FT_Glyph glyph, stroke;
		if(FT_Load_Glyph(face, cindex, FT_LOAD_DEFAULT)){
			nite::print("failed to load char '"+nite::toStr((char)i)+"' for font '"+filename+"'");
			continue;
		}
		if(FT_Get_Glyph(face->glyph, &glyph)){
			nite::print("failed to load char '"+nite::toStr((char)i)+"' for font '"+filename+"'");
			continue;
		}
		if(thickness > 0.0f){
			if(FT_Get_Glyph(face->glyph, &stroke)){
				nite::print("failed to load char '"+nite::toStr((char)i)+"' for font '"+filename+"'");
				continue;
			}		
			// stroke
			FT_Glyph_StrokeBorder(&stroke, stroker, 0, 1);			
		}
		// render to bitmap
		FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
		FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);
		FT_BitmapGlyph bitmapStroke;
		if(thickness > 0.0f){
			FT_Glyph_To_Bitmap(&stroke, FT_RENDER_MODE_NORMAL, NULL, true);
			bitmapStroke = reinterpret_cast<FT_BitmapGlyph>(stroke);
		}
		
		Single buffer;

		// render filler alone
		if(thickness == 0.0f){
			buffer = Single();
			buffer.pure(bitmap->bitmap.width, bitmap->bitmap.rows, bitmap->bitmap.buffer);
		// render outline as a strong and blit on it the filler
		}else{
			buffer = Single();
			buffer.strong(bitmapStroke->bitmap.width, bitmapStroke->bitmap.rows, bitmapStroke->bitmap.buffer);
			buffer.blitz(bitmap->bitmap.width, bitmap->bitmap.rows, bitmap->bitmap.buffer);		
		}

		frags.push_back(buffer);
		auto biggest = thickness == 0.0f ? bitmap : bitmapStroke;

		// metrics
		proxy.glyphs[i].index.x = xcursor;
		proxy.glyphs[i].index.y = 0;
		proxy.glyphs[i].coors.x = biggest->bitmap.width;
		proxy.glyphs[i].coors.y = biggest->bitmap.rows;
		proxy.glyphs[i].orig.x = biggest->left;
		proxy.glyphs[i].orig.y = biggest->top; 
		proxy.glyphs[i].size.x = face->glyph->advance.x >> 6;
		proxy.glyphs[i].size.y = face->glyph->metrics.horiBearingY >> 6;
		// atlas info
		dims.x += biggest->bitmap.width + 2;
		dims.y = std::max((unsigned)dims.y, biggest->bitmap.rows);
		// cursor update
		xcursor += biggest->bitmap.width + 2;
		FT_Done_Glyph(glyph);
		if(thickness > 0.0f){
			FT_Done_Glyph(stroke);
		}
	}
	proxy.vertAdvance = face->size->metrics.height >> 6;

	// compose atlas
	// TODO: might use a more complex algorithm than just a very long texture
	// specially if we ever get to implement unicode
	GLuint atlas;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &atlas);
	glBindTexture(GL_TEXTURE_2D, atlas);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dims.x, dims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);	
	xcursor = 0;
	for(int i = 0; i < frags.size(); ++i){
		auto &frag = frags[i];
		glTexSubImage2D(GL_TEXTURE_2D, 0, xcursor, 0, frag.w, frag.h, GL_RGBA, GL_UNSIGNED_BYTE, frag.buffer);
		xcursor += frag.w + 2;
		frag.clear();
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	// finish up
	proxy.hash = fileHash;
	proxy.atlasSize = dims;
	proxy.path = path;
	proxy.size = size;
	proxy.empty = false;
	proxy.atlas = atlas;

	nite::print("loaded font '"+filename+"' size: "+nite::toStr(size));

	// clean up
	if(thickness > 0.0f){
		FT_Stroker_Done(stroker);
	}
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

int nite::Font::getFontSize(){
	if(objectId <= -1) return 0;
	return fontList[objectId].size;
}

String nite::Font::getFilename(){
	if(objectId <= -1) return "";
	return fontList[objectId].path;
}

nite::Font::Font(){
	objectId = -1;
	scale = nite::Vec2(1, 1);
	smooth = false;
	shadow = true;
}

nite::Font::Font(const String &path, unsigned size, float outlineThickness){
	objectId = -1;
	scale = nite::Vec2(1, 1);
	shadow = false;
	smooth = true;
	load(path, size, outlineThickness);
}

void nite::Font::unload(){
	if(objectId <= -1) return;
	for(unsigned i=0; i<fontList[objectId].owners.size(); i++){
		if (fontList[objectId].owners[i] == this){
			fontList[objectId].owners.erase(fontList[objectId].owners.begin()+i);
			break;
		}
	}
	if (fontList[objectId].owners.size() == 0 && !fontList[objectId].stick){
		fontList[objectId].clear();
		fontList[objectId].empty = true;
	}
  	objectId = -1;
}

nite::Font::~Font(){
	unload();
}

nite::Font& nite::Font::operator= (const nite::Font &other){
	if(other.objectId <= -1) return *this;
	unload();
	scale = other.scale;
	shadowColor = other.shadowColor;
	shadow = other.shadow;
	objectId = other.objectId;
	fontList[objectId].owners.push_back(this);
	return *this;
}

static GLuint lastBind = 0;

void flushFont(){
	lastBind = 0;
}

void flushTexture();

static void drawText(nite::Renderable *object){
	if(!renText) return;
	flushTexture();
	nite::RenderableFont &obj = *(nite::RenderableFont*)object;
	unsigned Font = obj.objectId;
	if (fontList[Font].empty) return;
	GLint currentBind = fontList[Font].atlas;
	float dx = 0;
	float dy = 0;
	glEnable(GL_TEXTURE_2D);
	glColor4f(obj.color.r, obj.color.g, obj.color.b, obj.color.a);
	glBindTexture(GL_TEXTURE_2D, fontList[Font].atlas);
	if(currentBind != lastBind) // avoid re-binding the last texture.(an optimization, supposedly)
		glBindTexture(GL_TEXTURE_2D, currentBind);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	nite::Vec2 offset = obj.target == nite::RenderTargetDummy ? nite::Vec2(0.0f) : nite::getRenderOffset();
	glPushMatrix();
	glTranslatef((int)(obj.position.x - nite::getViewX(obj.target) + offset.x), (int)(obj.position.y - nite::getViewY(obj.target) + offset.y), 0.f);
	glRotatef((int)obj.angle, 0.0, 0.0, 1.0);
	for(int i = 0; i < obj.programs.size(); ++i){
		glUseProgram(obj.programs[i]->id);
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
	nite::Vec2 origin(obj.origin.x*obj.ref->getWidth(obj.text)*obj.scale.x, obj.origin.y*obj.ref->getRealHeight(obj.text)*obj.scale.y);
	if(obj.maxChars > 0 && obj.text.size() > obj.maxChars){
		if(!obj.autobreak){
			obj.text = obj.text.substr(0, obj.maxChars - 3);
			obj.text += "...";
		}else{
			int count = 0;
			auto words = nite::split(obj.text, ' ');
			if(words.size() > 1){
				String longest = "";
				String current = "";
				float base = obj.ref->getHeight("A");
				float height = base;
				obj.text = "";
				for(int i = 0; i < words.size(); ++i){
					count += words[i].length();
					obj.text += words[i];
					current += words[i];
					if(i < words.size()-1){
						obj.text += " ";
						current += " ";
						++count;
					}
					if(count > obj.maxChars && i < words.size()-1){
						if(current.length() > longest.length()){
							longest = current;
						}
						current = "";
						count = 0;
						obj.text += "\n";
						height += base;
					}
				}
				if(longest.length() == 0){
					longest = obj.text;
				}
				origin.set(obj.origin.x*obj.ref->getWidth(longest)*obj.scale.x, obj.origin.y*height*obj.scale.y);
			}
		}
	}
	// use rounded numbers to avoid atlas artifacts
	obj.origin.set((int)origin.x, (int)origin.y);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	for(unsigned i=0; i<obj.text.size(); i++){
		char current =  obj.text[i];
		float sX = obj.scale.x - (1.0f - 1.0f/SCALING);
		float sY = obj.scale.y - (1.0f - 1.0f/SCALING);
		if (current == '\n'){dx = 0; dy += obj.ln != -1.0f ? obj.ln : fontList[Font].vertAdvance; continue;}
		if (current == '\t'){dx += fontList[Font].glyphs['A'].size.x*3.0; continue;}
		float &xxi = fontList[Font].glyphs[current].index.x;
		float &yyi = fontList[Font].glyphs[current].index.y;
		float &wwi = fontList[Font].glyphs[current].coors.x;
		float &hhi = fontList[Font].glyphs[current].coors.y;
		float xi = dx+fontList[Font].glyphs[current].orig.x*sX;
		float yi = dy+fontList[Font].glyphs['A'].coors.y*sY-fontList[Font].glyphs[current].size.y*sY;
		GLfloat texBox[] = {
			xxi / fontList[Font].atlasSize.x,							yyi / fontList[Font].atlasSize.y,
			(xxi + wwi) / fontList[Font].atlasSize.x,			yyi / fontList[Font].atlasSize.y,
			(xxi + wwi) / fontList[Font].atlasSize.x,			(yyi + hhi) / fontList[Font].atlasSize.y,
			xxi / fontList[Font].atlasSize.x,							(yyi + hhi) / fontList[Font].atlasSize.y
		};
		if(obj.shadow){
			glColor4f(obj.shadowColor.r, obj.shadowColor.g, obj.shadowColor.b, obj.color.a);
			GLfloat shadowBox[] = {
				(-obj.origin.x+xi)+obj.shadowOffset.x, 									(-obj.origin.y+yi)+obj.shadowOffset.y,
				(wwi*sX-obj.origin.x+xi)+obj.shadowOffset.x, 	(-obj.origin.y+yi)+obj.shadowOffset.y,
				(wwi*sX-obj.origin.x+xi)+obj.shadowOffset.x, 	(hhi*sY-obj.origin.y+yi)+obj.shadowOffset.y,
				(-obj.origin.x+xi)+obj.shadowOffset.x,									(hhi*sY-obj.origin.y+yi)+obj.shadowOffset.y,
			};
			glVertexPointer(2, GL_FLOAT, 0, shadowBox);
			glTexCoordPointer(2, GL_FLOAT, 0, texBox);
			glDrawArrays(GL_QUADS, 0, 4);
			glColor4f(obj.color.r, obj.color.g, obj.color.b, obj.color.a);
		}
		GLfloat Box[] = {
			(-obj.origin.x+xi), 		(-obj.origin.y+yi),
			(wwi*sX-obj.origin.x+xi), 	(-obj.origin.y+yi),
			(wwi*sX-obj.origin.x+xi), 	(hhi*sY-obj.origin.y+yi),
			(-obj.origin.x+xi),		(hhi*sY-obj.origin.y+yi),
		};
		glVertexPointer(2, GL_FLOAT, 0, Box);
		glTexCoordPointer(2, GL_FLOAT, 0, texBox);
		glDrawArrays(GL_QUADS, 0, 4);
		dx += fontList[Font].glyphs[current].size.x*sX;
	}
	//glBindTexture(GL_TEXTURE_2D, 0);
	glDisableClientState( GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	lastBind = currentBind;
	glUseProgram(0);
	obj.programs.clear();
}

nite::RenderableFont * nite::Font::draw(const String &text, const nite::Vec2 &P){
	return draw(text, P.x, P.y);
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


nite::RenderableFont *nite::Font::draw(const String &text, float x, float y){
	if(objectId <= -1) return NULL;
	// if(isClippingOut(x, y, 25, 25, 0.0, 0.0)) return NULL;
	nite::RenderableFont *obj = new nite::RenderableFont();
	obj->text = text;
	obj->position.x = x;
	obj->position.y = y;
	obj->objectId = objectId;
	obj->function = &drawText;
	obj->color = nite::getColor();
	obj->angle = 0;
	obj->ref = this;
	obj->ln = ln;
	obj->smooth = smooth;
	obj->scale = scale;
	obj->avgHeight = this->getHeight(text);
	obj->origin = nite::Vec2(0, 0);
	obj->shadow = false;
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableFont *nite::Font::draw(const String &text, const nite::Vec2 &P, const nite::Vec2 &orig, float angle){
	return draw(text, P.x, P.y, orig.x, orig.y, angle);
}

nite::RenderableFont *nite::Font::draw(const String &text, const nite::Vec2 &P, const nite::Vec2 &orig, const nite::Vec2 &scale, float angle){
	return draw(text, P.x, P.y, orig.x, orig.y, scale.x, scale.y, angle);
}

nite::RenderableFont *nite::Font::draw(const String &text, float x, float y, float origX, float origY, float scaleX, float scaleY, float angle){
	if(objectId <= -1) return NULL;
	// if(isClippingOut(x, y, 25, 25, 0.0, 0.0)) return NULL;
	nite::RenderableFont *obj = new nite::RenderableFont();
	obj->text = text;
	obj->objectId = objectId;
	obj->function = &drawText;
	obj->color = nite::getColor();
	obj->angle = angle;
	obj->smooth = smooth;
	obj->ln = ln;
	obj->ref = this;
	obj->shadow = false;
	obj->avgHeight = this->getHeight(text);
	obj->scale = nite::Vec2(scaleX, scaleY);
	obj->origin.set(origX, origY);
	obj->position.set(x, y);
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableFont *nite::Font::draw(const String &text, float x, float y, float origX, float origY, float angle){
	if(objectId <= -1) return NULL;
	// if(isClippingOut(x, y, 25, 25, 0.0, 0.0)) return NULL;
	nite::RenderableFont *obj = new nite::RenderableFont();
	obj->text	= text;
	obj->objectId	= objectId;
	obj->function = &drawText;
	obj->color = nite::getColor();
	obj->angle = angle;
	obj->ref = this;
	obj->ln = ln;
	obj->smooth = smooth;
	obj->shadow = false;
	obj->scale = scale;
	obj->avgHeight = this->getHeight(text);
	obj->origin.set(origX, origY);
	obj->position.set(x, y);
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

void nite::Font::setSmooth(bool S){
	smooth = S;
}

float nite::Font::getWidth(const String &str){
	if(objectId <= -1) return 0;
	float s = 0.0f;
	float fs = 0.0f;
	for(unsigned i=0; i<str.length(); i++){
		if(str[i] == '\n'){
			fs = std::max(fs, s);
			s = 0.0f;
			continue;
		}
		s += fontList[objectId].glyphs[str[i]].size.x;
	}
	fs = std::max(fs, s);
	return fs > 0 ? fs / SCALING : 0;
}

float nite::Font::getRealHeight(const String &str){
	if(objectId <= -1) return 0;
	float t = 0.0f;
	for(int i = 0; i < str.size(); ++i){
		t = std::max(t, fontList[objectId].glyphs[str[i]].coors.y * SCALING);
	}
	return t;
}

float nite::Font::getHeight(){
	if(objectId <= -1) return 0;
	return fontList[objectId].vertAdvance / SCALING;
}

float nite::Font::getHeight(const String &str){
	if(objectId <= -1) return 0;
	float th = fontList[objectId].vertAdvance;
	float hdiff = fontList[objectId].glyphs['A'].size.y;
	int newln = 1;
	for(unsigned i=0; i<str.length(); i++){
		if(str[i] == '\n'){
			++newln;
			continue;
		}
	}
	return th * newln;
}