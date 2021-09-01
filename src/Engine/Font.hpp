#ifndef NITE_FONT_HPP
	#define NITE_FONT_HPP
	#include "Graphics.hpp"
	namespace nite {
		
		namespace TextAlign {
			enum TextAlign : int {
				CENTER,
				LEFT,
				RIGHT
			};
		}

		struct FontStyle {
			int size;
			float outline;
			nite::Color outlineColor;
			nite::Vec2 shadow;
			nite::Color shadowColor;
			FontStyle(){
				size = 14;
				outline = 0.0f;
				shadow.set(0.0f);
				shadowColor.set(0.0f);
				outlineColor.set(0.0f);
			}
		};		

		class Font;
		struct RenderableFont : public nite::Renderable {
			nite::FontStyle style;
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
			float horSpace;
			float avgHeight;
			float ln;
			int align;
			RenderableFont(){
				align = TextAlign::LEFT;
				horSpace = 0;
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
				void load(const String &path, const nite::FontStyle &style);
				Font();
				Font(const String &path, unsigned size, float outlineThickness = 0.0f);
				Font(const String &path, const nite::FontStyle &style);
				~Font();
				Font& operator= (const nite::Font &other);
				nite::RenderableFont *draw(const String &text, float x, float y);
				nite::RenderableFont *draw(const String &text, float x, float y, float origX, float origY, float angle);
				nite::RenderableFont *draw(const String &text, float x, float y, float origX, float origY, float scaleX, float scaleY, float angle);
				nite::RenderableFont *draw(const String &text, const nite::Vec2 &P, const nite::Vec2 &orig, const nite::Vec2 &scale, float angle);
				nite::RenderableFont *draw(const String &text, const nite::Vec2 &P, const nite::Vec2 &orig, float angle);
				nite::RenderableFont *draw(const String &text, const nite::Vec2 &P);
				void setTick(bool v);
				float getTickness();
				float getWidth(const String &str);
				float getHeight();
				float getHeight(const String &str);
				float getRealHeight(const String &str); // texture size
				void setScale(float x, float y);
				void setScale(const nite::Vec2 &s);
				void unload();
				void setSmooth(bool s);
				int getFontSize();
				String getFilename();
				nite::FontStyle style;
			private:	
				float thickness;
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
