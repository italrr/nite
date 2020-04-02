#ifndef NITE_FONT_HPP
	#define NITE_FONT_HPP
	#include "Graphics.hpp"
	namespace nite {
		class Font;
		struct RenderableFont : public nite::Renderable {
			String text;
			nite::Font *ref;
			unsigned objectId;
			bool smooth;
			float angle;
			nite::Vec2 origin;
			nite::Color shadowColor;
			bool shadow;
			nite::Vec2 shadowOffset;
			nite::Vec2 scale;
			int maxChars;
			bool autobreak;
			float avgHeight;
			float ln;
			RenderableFont(){
				maxChars = -1;
				autobreak = false;
			}
			void setShadow(const nite::Color &c){
				shadow = true;
				shadowColor	= c;
				shadowOffset = nite::Vec2(1, 1);
			}

			void setShadow(const nite::Color &c, const nite::Vec2 &offset){
				shadow = true;
				shadowColor	= c;
				shadowOffset = offset;
			}

			void setShadow(){
				shadow = true;
				shadowColor	= nite::Color(0, 0, 0, 1);
				shadowOffset = nite::Vec2(1, 1);
			}
		};

		class Font {
			public:
				bool isLoaded();
				void load(const String &path, unsigned size, float outlineThickness = 0.0f);
				Font();
				Font(const String &path, unsigned size, float outlineThickness = 0.0f);
				~Font();
				Font& operator= (const nite::Font &other);
				nite::RenderableFont *draw(const String &text, float x, float y);
				nite::RenderableFont *draw(const String &text, float x, float y, float origX, float origY, float angle);
				nite::RenderableFont *draw(const String &text, float x, float y, float origX, float origY, float scaleX, float scaleY, float angle);
				nite::RenderableFont *draw(const String &text, const nite::Vec2 &P, const nite::Vec2 &orig, const nite::Vec2 &scale, float angle);
				nite::RenderableFont *draw(const String &text, const nite::Vec2 &P, const nite::Vec2 &orig, float angle);
				nite::RenderableFont *draw(const String &text, const nite::Vec2 &P);
				void setTick(bool v);
				float getWidth(const String &str);
				float getHeight();
				float getHeight(const String &str);
				void setScale(float x, float y);
				void setScale(const nite::Vec2 &s);
				void unload();
				void setSmooth(bool s);
				int getFontSize();
				String getFilename();
			private:	
				int size;
				float ln;
				bool smooth;
				nite::Vec2 scale;
				nite::Color shadowColor;
				nite::Vec2 shadowOffset;
				bool shadow;
				int objectId;
		};
	}
#endif
