#include "Animation.hpp"
#include "Tools/Tools.hpp"
#include "Graphics.hpp"

nite::Animation::Animation(){

}

nite::Animation::Animation(const nite::Texture &tex){
	texture = tex;
}

nite::Animation::Animation(const String &path){
	texture.load(path);
}

nite::Animation::Animation(const String &path, const nite::Color &transparency){
	texture.load(path, transparency);
}

void nite::Animation::load(const String &path, const nite::Color &transparency){
	texture.load(path, transparency);
}

void nite::Animation::clear(){
	texture.unload();
}

void nite::Animation::load(const String &path){
	texture.load(path);
}

void nite::Animation::set(const nite::Texture &tex){
	texture = tex;
}

void nite::Animation::setScale(float x, float y){
	texture.setScale(x, y);
}

void nite::Animation::setScale(const nite::Vec2 &S){
	texture.setScale(S);
}

nite::Animation& nite::Animation::operator=(const nite::Animation &other){
	*this = other;
	texture = other.texture;
	for(unsigned i=0; animations.size(); ++i) animations[i].reset();
	return *this;
}

void nite::Animation::click(unsigned anim){
	if(anim >= animations.size()) return;
	nite::AnimationDescriptor &A = animations[anim];
	A.clicked = true;
}

void nite::Animation::setManualClicking(unsigned anim, bool set){
	if(anim >= animations.size()) return;
	nite::AnimationDescriptor &A = animations[anim];
	A.manual = set;
}

void nite::Animation::setFrame(unsigned anim, int frame){
	if(anim >= animations.size()) return;
	nite::AnimationDescriptor &A = animations[anim];
	A.manualFrame = frame;
}

int nite::Animation::getFrameNumber(unsigned anim){
	if(anim >= animations.size()) return 0;
	nite::AnimationDescriptor &A = animations[anim];
	return A.index.size();
}

unsigned nite::Animation::getFrame(unsigned anim){
	nite::AnimationDescriptor &Anim = animations[anim];
	float frame = Anim.manual ? 0 : Anim.current;
	frame += Anim.clicked ? 1 : (Anim.manual ? 0 : (1.0f - nite::pow(1.0f - Anim.speed, nite::getDelta() * nite::getTimescale() * 0.077f)));
	frame = frame >= Anim.index.size() ? nite::round(frame) % Anim.index.size() : frame;
	Anim.current = frame;
	Anim.clicked = false;
	if(Anim.manualFrame != -1){
		frame = Anim.manualFrame;
	}
	Anim.manualFrame = -1;
	return frame;
}

void nite::Animation::setSpeed(unsigned anim, float speed){
	if(anim >= animations.size()) return;
	nite::AnimationDescriptor &A = animations[anim];
	A.speed = speed;
}

nite::RenderableTextureT * nite::Animation::draw(unsigned anim, float x, float y){
	if(anim >= animations.size()) return NULL;
	nite::AnimationDescriptor &A = animations[anim];
	texture.setRegion(animations[anim].index[getFrame(anim)], A.frameSize);
	return texture.draw(x, y);
}

nite::RenderableTextureT * nite::Animation::draw(unsigned anim,float x, float y, float angle){
	if(anim >= animations.size()) return NULL;
	texture.setRegion(animations[anim].index[getFrame(anim)], animations[anim].frameSize);
	return texture.draw(x, y, angle);
}

nite::RenderableTextureT * nite::Animation::draw(unsigned anim,const nite::Vec2 &P){
	if(anim >= animations.size()) return NULL;
	texture.setRegion(animations[anim].index[getFrame(anim)], animations[anim].frameSize);
	return texture.draw(P);
}

nite::RenderableTextureT * nite::Animation::draw(unsigned anim,const nite::Vec2 &P, float angle){
	if(anim >= animations.size()) return NULL;
	texture.setRegion(animations[anim].index[getFrame(anim)], animations[anim].frameSize);
	return texture.draw(P, angle);
}

nite::RenderableTextureT * nite::Animation::draw(unsigned anim,const nite::Vec2 &P, const nite::Vec2 &S, const nite::Vec2 &orig, float angle){
	if(anim >= animations.size()){
		return NULL;
	}
	texture.setRegion(animations[anim].index[getFrame(anim)], animations[anim].frameSize);
	return texture.draw(P, S, orig, angle);
}

nite::RenderableTextureT * nite::Animation::draw(unsigned anim,float x, float y, float w, float h, float origX, float origY, float angle){
	if(anim >= animations.size()){
		return NULL;
	}
	texture.setRegion(animations[anim].index[getFrame(anim)], animations[anim].frameSize);
	return texture.draw(x, y, w, h, origX, origY, angle);
}

unsigned nite::Animation::add(float x, float y, float w, float h, unsigned n, float speed, bool vertical){
	return add(x, y, w, h, n, speed, vertical, false);
} 

unsigned nite::Animation::add(const nite::Vec2 &ss, const Jzon::Node &node){
	nite::Vec2 index(node.get("x").toFloat(), node.get("y").toFloat());
	int n = node.get("n").toInt();
	bool reversed = node.get("reverse").toBool(false);
	bool vertical = node.get("vert").toBool(false);
	float speed = node.get("spd").toFloat();
	return add(ss.x * index.x, ss.y * index.y, ss.x, ss.y, n, speed, vertical, reversed);
}

unsigned nite::Animation::add(float x, float y, float w, float h, unsigned n, float speed, bool vertical, bool reversed){
	nite::AnimationDescriptor current;
	current.speed 	= speed;
	current.frames 	= n;
	current.frameSize.set(w, h);
	for(unsigned i=0; i<n; ++i){
		current.index.push_back(nite::Vec2(x, y));
		x = vertical ? x 	: x+w;
		y = vertical ? y+h 	: y;
	}
	if(reversed){
		for(unsigned i=current.index.size()-1; i>0; i--){
			current.index.push_back(nite::Vec2(current.index[i]));
		}
	}
	animations.push_back(current);
	return animations.size()-1;
}
