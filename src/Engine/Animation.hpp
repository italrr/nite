#ifndef NITE_ANIMATION
	#define NITE_ANIMATION
	#include "Texture.hpp"
	#include "Tools/Tools.hpp"
	namespace nite {
		struct AnimationDescriptor {
			float speed;
			float current;
			int frame;
			bool manual;
			Vector<nite::Vec2> index;
			nite::Vec2 frameSize;
			unsigned frames;
			bool clicked;
			int manualFrame;
			AnimationDescriptor(){
				reset();
				manualFrame = -1;
				manual = false;
				clicked = false;
			}
			void reset(){
				frame = -1;
				current = 0;
			}
		};
		class Animation {
			public:
				Animation();
				Animation(const nite::Texture &tex);
				Animation(const String &path);
				Animation(const String &path, const nite::Color &transparency);
				void load(const String &path);
				void load(const String &path, const nite::Color &transparency);
				void set(const nite::Texture &tex);
				nite::RenderableTextureT *draw(unsigned anim, float x, float y);
				nite::RenderableTextureT *draw(unsigned anim, float x, float y, float angle);
				nite::RenderableTextureT *draw(unsigned anim, const nite::Vec2 &P);
				nite::RenderableTextureT *draw(unsigned anim, const nite::Vec2 &P, float angle);
				nite::RenderableTextureT *draw(unsigned anim, const nite::Vec2 &P, const nite::Vec2 &S, const nite::Vec2 &orig, float angle);
				nite::RenderableTextureT *draw(unsigned anim, float x, float y, float w, float h, float origX, float origY, float angle);
				unsigned add(float x, float y, float w, float h, unsigned n, float speed, bool vertical);
				unsigned add(float x, float y, float w, float h, unsigned n, float speed, bool vertical, bool reversed);
				unsigned add(const nite::Vec2& ss, const Jzon::Node &node);
				void clear();
				Animation& operator= (const Animation &other);
				nite::Texture texture;
				void click(unsigned anim);
				void setManualClicking(unsigned anim, bool set);
				int getFrameNumber(unsigned anim);
				void setFrame(unsigned anim, int frame);
				void setScale(float x, float y);
				void setScale(const nite::Vec2 &S);
				void setSpeed(unsigned anim, float speed);
				Vector<AnimationDescriptor> animations;
			private:
				nite::Vec2 scale;
				bool smooth;
				unsigned getFrame(unsigned anim);

				bool loaded;
		};
	}
#endif
