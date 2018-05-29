#ifndef NITE_UI
  #define NITE_UI

  #include "BaseUIComponent.hpp"
  #include "WindowUI.hpp"
  #include "TextUI.hpp"
  #include "ButtonUI.hpp"
  #include "CheckboxUI.hpp"  
  #include "PanelUI.hpp"
  #include <memory>

  namespace nite {
    
    struct UIMaster {

      Vector<std::shared_ptr<nite::BaseUIComponent>> components;

      void add(std::shared_ptr<nite::BaseUIComponent> comp);
      void update();
      void render();
      Vector<nite::BaseUIComponent*> removeQueue;
      void remove(nite::BaseUIComponent *comp);

      // builds a UI -anything- from a JSON file      
      std::shared_ptr<nite::BaseUIComponent> build(const String &path, const Dict<String, nite::ListenerLambda> &listeners);    
      std::shared_ptr<nite::BaseUIComponent> build(const String &path); 
    }; 
  } 

#endif