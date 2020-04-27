#ifndef NITE_ICON_UI_HPP
  #define NITE_ICON_UI_HPP

  #include "BaseUIComponent.hpp"
  #include "../Tools/Tools.hpp"
  #include "../Shader.hpp"
  #include "../Texture.hpp"
  #include "../Font.hpp"  

  namespace nite {  

    class IconUI : public nite::BaseUIComponent {
      private:
        nite::Texture source;
        nite::Texture empty;
        nite::Batch batch;
        void rerender();
        void recalculate();
        void defaultInit();
        bool toRerender;
        void onHover();
        void onUnhover();        
        void onClick();
        nite::ListenerLambda onClickMethod;
        nite::ListenerLambda onHoverMethod;
        nite::ListenerLambda onUnhoverMethod;                                              
        void recalculateFromHead();
        nite::Vec2 iconSize;
        nite::Vec2 iconPosition;
        nite::Vec2 textPosition;
        nite::Color textColor;
        nite::Color shadowColor;
        nite::Vec2 shadowOffset;        
        nite::Font font;
        int fontSize;
        String text;
        int index;
      public: 
        void setShadowColor(const nite::Color &color);
        nite::Color getShadowColor();
        void setShadowOffset(const nite::Vec2 &offset);      
        nite::Vec2 getShadowOffset();
        void setFontSize(int s);
        int getFontSize();
        void setFont(const nite::Font &font);
        nite::Font getFont();
        nite::Color getTextColor();
        void setTextColor(const nite::Color &c);
        void setText(const String &text);
        String getText();
        void setTextPosition(const nite::Vec2 &p);
        nite::Vec2 getTextPosition();
        void setSource(const nite::Texture &bgi);
        nite::Texture getSource();
        nite::Vec2 getIconSize();
        void setIconSize(const nite::Vec2 &iconSize);
        void setOnUnhover(nite::ListenerLambda onUnhover);                  
        void setOnClick(nite::ListenerLambda onClick);
        void setOnHover(nite::ListenerLambda onHover);
        void setIndex(int index);
        int getIndex();
        IconUI(const nite::Vec2 &size, const nite::Vec2 &iconSize, const nite::Texture &source, int index);
        IconUI();
        void accommodate(float w, float h);
        void setSize(const nite::Vec2 &size);
        nite::Vec2 computeSize();
        void onCreate();
        void update();
        void render(const nite::Vec2 &offset);
        void setBackgroundColor(const nite::Color &color);
        nite::Color getBackgroundColor();
    };

  }

#endif