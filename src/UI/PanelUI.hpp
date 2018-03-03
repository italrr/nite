#ifndef NITE_PANEL_UI
  #define NITE_PANEL_UI

  #include "BaseUIComponent.hpp"
  #include "../Engine/Tools/Tools.hpp"
  #include "../Engine/Shader.hpp"
  #include "../Engine/Texture.hpp"
  #include "../Engine/Font.hpp"  

  namespace nite {  

    class PanelUI : public nite::BaseUIComponent {
      private:
        nite::Texture uiBasicTexture;
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
      public: 
        void setOnUnhover(nite::ListenerLambda onUnhover);                  
        void setOnClick(nite::ListenerLambda onClick);
        void setOnHover(nite::ListenerLambda onHover);
        PanelUI(const nite::Vec2 &size);
        PanelUI();
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