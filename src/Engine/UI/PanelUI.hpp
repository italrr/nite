#ifndef NITE_PANEL_UI_HPP
  #define NITE_PANEL_UI_HPP

  #include "BaseUIComponent.hpp"
  #include "../Tools/Tools.hpp"
  #include "../Shader.hpp"
  #include "../Texture.hpp"
  #include "../Font.hpp"  

  namespace nite {  

    struct PanelUI : public nite::BaseUIComponent {
        float borderThickness;
        nite::Color borderColor;
        nite::Shader uiShader;
        nite::Texture uiBasicTexture;
        nite::Texture uiBackgroundImage;
        nite::Batch batch;
        void rerender();
        void recalculate();
        void setBorderThickness(float tn);
        void setBorderColor(const nite::Color &color);
        void defaultInit();
        bool toRerender;
        void onHover();
        void onUnhover();        
        void onClick();
        nite::ListenerLambda onClickMethod;
        nite::ListenerLambda onHoverMethod;
        nite::ListenerLambda onUnhoverMethod;                                              
        void recalculateFromHead(); 
        void setBackgroundImage(const nite::Texture &bgi);
        nite::Texture getBackgroundImage();      
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
        void render(const nite::Vec2 &offset);
        void setBackgroundColor(const nite::Color &color);
        nite::Color getBackgroundColor();
    };

  }

#endif