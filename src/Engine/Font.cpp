#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>
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

struct Word {
	String word;
	float width;
	float height;
	float dx;
	float dy;
};


struct Cursor {
	float x;
	float y;
	Cursor(){ x = y = 0; }
	Cursor operator += (const Cursor &other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
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
	int bw, bh;
	unsigned char *buffer;
	size_t size;
	Single(int w, int h, unsigned char *src){
		pure(w, h, src);
	}
	// pure makes a base with the exact same level per channel
	void pure(int w, int h, unsigned char* src){
		int channels = 2;
		this->w = w;
		this->h = h;
		this->bw = w;
		this->bh = h;
		size = w * h * channels;
		buffer = new unsigned char[size];
		memset(buffer, 0, size);
		for(int i = 0; i < w*h; ++i){
			buffer[i*channels + 0] = src[i];
		}
	}
	// strong makes a base with strong opposite
	void strong(int x, int y, int w, int h, int bw, int bh, unsigned char* src){
		int channels = 2;
		this->w = w;
		this->h = h;
		this->bw = bw == 0 ? w : bw;
		this->bh = bh == 0 ? h : bh;
		size = this->bw * this->bh * channels;
		buffer = new unsigned char[size];
		memset(buffer, 0, size);
		int offsetx = nite::round((float)(this->w - w) / 2.0f) + x;
		int offsety = nite::round((float)(this->h - h) / 2.0f) + y;		
		// for(int i = 0; i < w*h; ++i){
		// 	// unsigned char v = src[i] > 0 ? 0 : 255;
		// 	// buffer[i*channels + 0] = v;
		// 	// buffer[i*channels + 1] = v;
		// 	// buffer[i*channels + 2] = v;
		// 	// buffer[i*channels + 3] = src[i];
		// 	buffer[i*channels + 1] = src[i];
		// }
		for(int y = 0; y < h; ++y){
			for(int x = 0; x < w; ++x){
				int i = y * w  + x;
				int j = (y + offsety)  * this->bw + x + offsetx;		
				buffer[j*channels + 1] = src[i];
			}
		}		
	}	
	void blitz(int x, int y, int w, int h, int bw, int bh, unsigned char* src){
		// blitz expects a smaller bitmap
		int channels = 2;
		int offsetx = nite::round((float)(this->w - w) / 2.0f) + x;
		int offsety = nite::round((float)(this->h - h) / 2.0f) + y;
		for(int y = 0; y < h; ++y){
			for(int x = 0; x < w; ++x){
				int i = y * w  + x; //src
				int j = (y + offsety)  * (bw == 0 ? this->w : bw) + x + offsetx; // target
				// buffer[j*4 + 0] = src[i];
				// buffer[j*4 + 1] = src[i];
				// buffer[j*4 + 2] = src[i];
				// buffer[j*4 + 3] = (buffer[j*4 + 3] + src[i]); // disregard alpha			
				buffer[j*channels + 0] = src[i];
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

void nite::Font::load(const String &path, unsigned size, float thickness){
	nite::FontStyle style;
	style.outline = thickness;
	style.size = size;
	load(path, style);
}

// TODO: Unicode
void nite::Font::load(const String &path, const nite::FontStyle &style){
	if(style.size <= 0){
		return;
	}
	FT_Library library;
	FT_Face face;
	String filename = path;
	this->size = style.size * SCALING;
	this->style = style;
	this->thickness = style.outline;
	ln = -1.0f;
	// thickness = 0.0f;

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
	String pathAndSize = path+nite::toStr(style.size);
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


	if(FT_Set_Char_Size(face, 0, style.size << 6, 96, 96)){
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
	float singleSpacing = 6.0f;
	// render characters
	for(int i = ASCII_MIN; i < ASCII_MAX; i++){
		FT_Load_Char(face, i, FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL);//FT_Get_Char_Index(face, i);
		FT_Glyph glyph, stroke;
		// if(FT_Load_Glyph(face, cindex, FT_LOAD_DEFAULT)){
		// 	nite::print("failed to load char '"+nite::toStr((char)i)+"' for font '"+filename+"'");
		// 	continue;
		// }

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
		nite::Vec2 bmapSize = nite::Vec2(bitmap->bitmap.width, bitmap->bitmap.rows);
		// render filler alone
		if(thickness == 0.0f && style.shadow.x == 0.0f && style.shadow.y == 0.0f){
			buffer = Single();
			buffer.pure(bitmap->bitmap.width, bitmap->bitmap.rows, bitmap->bitmap.buffer);
		// render outline as a strong and blit on it the filler
		}else
		if(style.shadow.x > 0.0f || style.shadow.y > 0.0f){
			bmapSize = nite::Vec2(bitmap->bitmap.width + style.shadow.x, bitmap->bitmap.rows + style.shadow.y);
			buffer = Single();
			buffer.strong(style.shadow.x, style.shadow.y, bitmap->bitmap.width, bitmap->bitmap.rows, bmapSize.x, bmapSize.y, bitmap->bitmap.buffer);
			buffer.blitz(0,  0, bitmap->bitmap.width, bitmap->bitmap.rows, bmapSize.x, bmapSize.y, bitmap->bitmap.buffer);
		}else{
			bmapSize = nite::Vec2( bitmapStroke->bitmap.width, bitmapStroke->bitmap.rows);
			buffer = Single();
			buffer.strong(0, 0, bitmapStroke->bitmap.width, bitmapStroke->bitmap.rows, 0, 0, bitmapStroke->bitmap.buffer);
			buffer.blitz(0, 0, bitmap->bitmap.width, bitmap->bitmap.rows, 0, 0, bitmap->bitmap.buffer);		
		}

		frags.push_back(buffer);
		auto biggest = thickness == 0.0f ? bitmap : bitmapStroke;

		// metrics
		proxy.glyphs[i].index.x = xcursor;
		proxy.glyphs[i].index.y = 0;
		proxy.glyphs[i].coors.x = bmapSize.x;
		proxy.glyphs[i].coors.y = bmapSize.y;
		proxy.glyphs[i].orig.x = biggest->left;
		proxy.glyphs[i].orig.y = biggest->top; 
		proxy.glyphs[i].size.x = face->glyph->advance.x >> 6;
		proxy.glyphs[i].size.y = face->glyph->metrics.horiBearingY >> 6;
		// atlas info
		dims.x += bmapSize.x + singleSpacing;
		dims.y = std::max((unsigned)dims.y, (unsigned)bmapSize.y);
		// cursor update
		xcursor += bmapSize.x + singleSpacing;
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
	
	// if(thickness > 0){
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, dims.x, dims.y, 0, GL_RG, GL_UNSIGNED_BYTE, 0);	
	// }else{
	// 	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dims.x, dims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);			
	// }
	
	xcursor = 0;
	for(int i = 0; i < frags.size(); ++i){
		auto &frag = frags[i];
		// if(thickness > 0){
			glTexSubImage2D(GL_TEXTURE_2D, 0, xcursor, 0, frag.bw, frag.bh, GL_RG, GL_UNSIGNED_BYTE, frag.buffer);
		// }else{
		// 	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		// 	glTexSubImage2D(GL_TEXTURE_2D, 0, xcursor, 0, frag.w, frag.h, GL_RGBA, GL_UNSIGNED_BYTE, frag.buffer);
		// }
		xcursor += frag.bw + singleSpacing;
		frag.clear();
	}
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		
	
	// finish up
	proxy.hash = fileHash;
	proxy.atlasSize = dims;
	proxy.path = path;
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

nite::Font::Font(const String &path, const nite::FontStyle &style){
	objectId = -1;
	scale = nite::Vec2(1, 1);
	shadow = false;
	smooth = true;
	load(path, style);	
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
	style = other.style;
	scale = other.scale;
	shadowColor = other.shadowColor;
	shadow = other.shadow;
	thickness = other.thickness;
	objectId = other.objectId;
	fontList[objectId].owners.push_back(this);
	return *this;
}

static GLuint lastBind = 0;

void flushFont(){
	lastBind = 0;
}

void flushTexture();

struct Token {
	int type;
	nite::Color color;
	int position;
	String value;
	bool found;
	Token(){
		found = false;
	}
};

namespace TokenType {
	enum TokenType : int {
		UNDEFINED,
		COLOR_SET,
		COLOR_RESET
	};
	static int type(const String &name){
		if(name == "cs"){
			return COLOR_SET;
		}else
		if(name == "cr"){
			return COLOR_RESET;
		}else{
			return UNDEFINED;
		}
	}
}

static Token fetchToken(int stPos, const String &input){
	String found = "";
	Token token;
	for(int i = stPos; i < input.size(); ++i){
		if(i < input.size()-1 && input[i] == '$' && input[i+1] == '['){
			int end = input.find("]", i+1);
			if(end != std::string::npos){
				found = input.substr(i+2, end-i-2);
				token.position = end;
				break;
			}
		}
	}
	if(found.size() > 0){
		int colon = found.find(":");
		if(colon != std::string::npos){
			String first = found.substr(0, colon);
			String value = found.substr(colon+1, found.length()-colon);
			token.type = TokenType::type(first);
			token.value = value;
		}else{
			token.type = TokenType::type(found);
		}
		token.found = true;
	}
	return token;
}

static nite::Shader fontRenderShader;

static void drawText(nite::Renderable *object){
	if(!renText) return;
	if(!fontRenderShader.isLoaded()){
		fontRenderShader.load("data/shaders/font_render_f.glsl", "data/shaders/font_render_v.glsl");
	}

	flushTexture();
	nite::RenderableFont &obj = *(nite::RenderableFont*)object;
	unsigned Font = obj.objectId;
	if (fontList[Font].empty) return;
	GLint currentBind = fontList[Font].atlas;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fontList[Font].atlas);
	if(currentBind != lastBind) // avoid re-binding the last texture.(an optimization, supposedly)
		glBindTexture(GL_TEXTURE_2D, currentBind);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	nite::Vec2 offset = obj.target == nite::RenderTargetDummy ? nite::Vec2(0.0f) : nite::getRenderOffset();
	glPushMatrix();
	glTranslatef((int)nite::round(obj.position.x - nite::getViewX(obj.target) + offset.x), (int)nite::round(obj.position.y - nite::getViewY(obj.target) + offset.y), 0.f);
	glRotatef((int)obj.angle, 0.0, 0.0, 1.0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	auto applyTextColor = [&](const nite::Color &fillColor, const nite::Color &outlineColor){
		// return;
		glUseProgram(fontRenderShader.getProgram());
		float v[3];
		int olcuniform = glGetUniformLocation(fontRenderShader.getProgram(), "outline_col");
		v[0] = outlineColor.r;
		v[1] = outlineColor.g;
		v[2] = outlineColor.b;
		glUniform3fv(olcuniform, 1, v);
		int fcuniform = glGetUniformLocation(fontRenderShader.getProgram(), "fill_col");
		v[0] = fillColor.r;
		v[1] = fillColor.g;
		v[2] = fillColor.b;
		glUniform3fv(fcuniform, 1, v);
	};
	applyTextColor(obj.color, obj.style.outlineColor);

	auto startBatch = [&](){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
	};

	auto endBatch = [&](){
		glDisableClientState( GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	};

	startBatch();

	// use rounded numbers to avoid atlas artifacts

	auto wordList = nite::split(obj.text, ' ');

	Vector<Word> words;
	float sX = obj.scale.x - (1.0f - 1.0f/SCALING);
	float sY = obj.scale.y - (1.0f - 1.0f/SCALING);	
	float totalWidth = 0;
	float vertAdv = fontList[Font].vertAdvance * sY;
	float horAdv = fontList[Font].glyphs[' '].size.x * sX;	
	for(int i = 0; i < wordList.size(); ++i){
		Word word;
		word.word = wordList[i];
		int w = 0, h = 0;
		for(unsigned j = 0; j < wordList[i].size(); j++){
			char current =  wordList[i][j];
			w += fontList[Font].glyphs[current].size.x*sX;
			h += fontList[Font].glyphs[current].size.y*sY;
		}
		totalWidth += w;
		word.width = w;
		word.height = h;
		words.push_back(word);
	}

	// add spaces to totalWidth
	for(int i = 0; i < wordList.size()-1; ++i){
		totalWidth += fontList[Font].glyphs[' '].size.x*sX;
	}

	auto render = [&](const String &word, Cursor &from, nite::Vec2 &origin){
		from.x = nite::round(from.x);
		from.y = nite::round(from.y);

		for(unsigned j = 0; j < word.size(); j++){
			char current =  word[j];

			if(j < word.size()-1 && word[j] == '$' && word[j+1] == '['){
				auto token = fetchToken(j, word);
				if(token.found){
					// process
					if(token.type != TokenType::UNDEFINED){
						switch(token.type){
							case TokenType::COLOR_SET: {
								auto color = nite::Color(token.value);
								// glColor4f(color.r, color.g, color.b, 1.0f);
								applyTextColor(color, obj.style.outlineColor);
							} break;
							case TokenType::COLOR_RESET: {
								applyTextColor(obj.color, obj.style.outlineColor);
							} break;
						}
					}
					j = token.position;
					continue;
				}
			}

			// origin.set(0.0f);
			if (current == '\n'){ from.x = 0; from.y += obj.ln != -1.0f ? obj.ln : vertAdv; continue; }
			if (current == '\t'){ from.x += fontList[Font].glyphs['A'].size.x*3.0f; continue; }
			float &xxi = fontList[Font].glyphs[current].index.x;
			float &yyi = fontList[Font].glyphs[current].index.y;
			float &wwi = fontList[Font].glyphs[current].coors.x;
			float &hhi = fontList[Font].glyphs[current].coors.y;
			float xi = from.x + fontList[Font].glyphs[current].orig.x*sX;
			float yi = from.y + fontList[Font].glyphs['A'].coors.y*sY-fontList[Font].glyphs[current].size.y*sY;
			GLfloat texBox[] = {
				xxi / fontList[Font].atlasSize.x,					yyi / fontList[Font].atlasSize.y,
				(xxi + wwi) / fontList[Font].atlasSize.x,			yyi / fontList[Font].atlasSize.y,
				(xxi + wwi) / fontList[Font].atlasSize.x,			(yyi + hhi) / fontList[Font].atlasSize.y,
				xxi / fontList[Font].atlasSize.x,					(yyi + hhi) / fontList[Font].atlasSize.y
			};
			GLfloat Box[] = {
				(-origin.x+xi), 		(-origin.y+yi),
				(wwi*sX-origin.x+xi), 	(-origin.y+yi),
				(wwi*sX-origin.x+xi), 	(hhi*sY-origin.y+yi),
				(-origin.x+xi),			(hhi*sY-origin.y+yi),
			};
			glVertexPointer(2, GL_FLOAT, 0, Box);
			glTexCoordPointer(2, GL_FLOAT, 0, texBox);
			glDrawArrays(GL_QUADS, 0, 4);
			from.x += fontList[Font].glyphs[current].size.x*sX;
		}
	};
	Cursor current;
	auto resetAlign = [&](float totalWidth){
		switch(obj.align){
			case nite::TextAlign::RIGHT:
				current.x = obj.horSpace - totalWidth;
			break;
			case nite::TextAlign::CENTER:
				current.x = obj.horSpace*0.5f - totalWidth*0.5f;
			break;
			case nite::TextAlign::LEFT:
			default:
				current.x = 0;
			break;									
		}
	};	
	
	if(obj.horSpace > 0){
		resetAlign(totalWidth);
	}

	if(!obj.autobreak){
		nite::Vec2 origin(obj.origin.x*totalWidth, obj.origin.y*vertAdv);
		for(int i = 0; i < words.size(); ++i){
			auto &word = words[i];
			bool nl = false;
			if(obj.autobreak && obj.horSpace > 0 && current.x + word.width > obj.horSpace){
				current.y += vertAdv;
				resetAlign(totalWidth);
				nl = true;
			}
			render(word.word, current, origin);
			if(!nl && i < words.size()-1) render(" ", current, origin);			
		}
	}else{
		Vector<Vector<int>> lines;
		Vector<int> cl;
		Vector<float> lineW;
		float longestLine = 0;
		float c = 0;
		for(int i = 0; i < words.size(); ++i){
			if(c + horAdv + words[i].width > obj.horSpace){
				lineW.push_back(c);
				longestLine = std::max(c, longestLine);
				c = 0;
				lines.push_back(cl);
				cl.clear();
			}
			cl.push_back(i);
			c += words[i].width + horAdv;
		}
		longestLine = std::max(c, longestLine);
		lines.push_back(cl);
		lineW.push_back(c);
		longestLine = std::max(obj.horSpace, longestLine);
		nite::Vec2 origin((int)(obj.origin.x*longestLine), (int)(obj.origin.y*((float)lines.size()*vertAdv)));
		// obj.origin.set((int)origin.x, (int)origin.y);
		for(int i = 0; i < lines.size(); ++i){
			resetAlign(lineW[i]);
			for(int j = 0; j < lines[i].size(); ++j){
				auto &word = words[lines[i][j]];
				render(word.word, current, origin);
				if(j < lines[i].size()-1)render(" ", current, origin);
			}			
			current.y += vertAdv;
		}
	}

	//glBindTexture(GL_TEXTURE_2D, 0);
	endBatch();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	lastBind = currentBind;
	glUseProgram(0);
	obj.programs.clear();
}

float nite::Font::getTickness(){
	return thickness;
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
	obj->style = this->style;
	obj->ln = -1;
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
	obj->ln = -1;
	obj->ref = this;
	obj->style = this->style;
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
	obj->ln = -1;
	obj->style = this->style;
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

		if(i < str.size()-1 && str[i] == '$' && str[i+1] == '['){
			auto token = fetchToken(i, str);
			if(token.found){
				i = token.position;
				continue;
			}
		}

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