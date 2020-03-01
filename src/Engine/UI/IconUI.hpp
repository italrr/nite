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
        int index;
      public: 
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
        void render();
        void setBackgroundColor(const nite::Color &color);
        nite::Color getBackgroundColor();
    };

  }

#endif