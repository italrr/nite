#ifndef NITE_BASIC_UI_COMPONENT
  #define NITE_BASIC_UI_COMPONENT

  #include "../Engine/Graphics.hpp"
  #include "../Engine/Tools/Tools.hpp"
  #include <memory>

  namespace nite {

    struct BaseUIComponent;
    struct UIMaster;    

    struct LayoutSystemUI {
      
      nite::Vec2 size;
      virtual void recalculate(BaseUIComponent &head){

      }
    };

    namespace Layout {
      struct VBox : public nite::LayoutSystemUI {
        void recalculate(BaseUIComponent &head);
      };
      struct HBox : public nite::LayoutSystemUI {
        void recalculate(BaseUIComponent &head);
      };       
      struct Inline : public nite::LayoutSystemUI {
        void recalculate(BaseUIComponent &head);
      };         
    }

    static const String defaultFontUI = "data/font/Saniretro.ttf";
    static const float defaultFontRatio = 1.5f;
    static const float componentTranslationSpeed = 0.25f;

    struct BaseUIComponent {
      nite::Vec2 position;
      nite::Vec2 realPosition;
      nite::Vec2 size;
      nite::Vec2 margin;
      nite::Vec2 padding;
      nite::Vec2 relativePosition;
      nite::Color baseColor;
      nite::UIMaster *currentMaster;
      int zdepth;
      bool fillUpType;
      unsigned renderOnTarget;
      std::shared_ptr<nite::LayoutSystemUI> layout;
      Vector<std::shared_ptr<nite::BaseUIComponent>> children;
      BaseUIComponent *headComponent;
      float flex;
      bool solid;
      UInt64 uiCreatedTimeout;
      String componentName;
      bool uninteract;


      BaseUIComponent *getTopHeadComponent(){
        return headComponent == NULL ? this : headComponent->getTopHeadComponent();
      }

      BaseUIComponent *getHeadComponent(){
        return headComponent == NULL ? this : headComponent;
      }

      void setLayout(std::shared_ptr<nite::LayoutSystemUI> layout){
        this->layout = layout;
        recalculate();
      }

      void updateRelativePosition(const nite::Vec2 &pos){
        this->relativePosition.set(pos);
      }

      /*
        Layouts will call this on the Component when they're being
        arranged, but only if 'fillUpType' is true.

        accommodate must recompute and therefore,
        reconfigure the Component to expand itself
        within the w & h provided by the Layout.

        accommodate must also resolve overflowing.
      */
      virtual void accommodate(float w, float h){
        size.set(w - padding.x - margin.x, h - padding.y - margin.y);
      }

      BaseUIComponent(){
        padding.set(0.0f);
        margin.set(0.0f);
        size.set(16.0f);
        position.set(0.0f);
        baseColor.set(0.0f, 0.0f, 0.0f, 1.0f);
        zdepth = 0;
        flex = 0.0f;
        solid = false;
        fillUpType = false;
        layout = std::shared_ptr<nite::LayoutSystemUI>(new nite::Layout::Inline());
        headComponent = NULL;
        renderOnTarget = nite::RenderTargetUI;
        isHovered = false;
        currentMaster = NULL;
        uninteract = false;
      }

      void setFlex(float flex){
        this->flex = flex;
        recalculate();
      }

      void setInteractive(bool v){
        uninteract = !v;
        for(int i = 0; i < children.size(); ++i){
          children[i]->setInteractive(v);
        } 
      }

      virtual void defaultInit(){

      }
      
      virtual void renderDebugFrame(){
        
      }

      virtual void rerender(){

      }

      virtual void calculateSize(){

      }

      virtual void recalculate(){
        auto *head = getHeadComponent();
        computeSize();
        if(head == this) return;
        head->recalculate();
      }

      void add(std::shared_ptr<nite::BaseUIComponent> component);

      void destroy();    

      void clear(){
        for(int i = 0; i < children.size(); ++i){
          auto child = children[i];
          child->onDestroy();
          child->headComponent = NULL;
        }        
        children.clear();
        recalculate();
      }

      virtual nite::Vec2 computeSize(){
        return size + margin + padding;
      }

      void preinit(){
        uiCreatedTimeout = nite::getTicks();
      }
      
      void setMargin(const nite::Vec2 &size){
        this->margin.set(size);
        recalculate();
      }

      void setPadding(const nite::Vec2 &size){
        this->padding.set(size);
        recalculate();
      }

      bool isHovered;

      virtual void updateListeners();

      // Listeners
      virtual void onClick(){

      }

      virtual void onUnhover(){

      }

      virtual void onHover(){

      }

      virtual void onDestroy(){

      }

      virtual void onCreate(){

      }

      // Update
      virtual void beforeUpdate(){

      }
      virtual void update(){

      }
      virtual void afterUpdate(){

      }

      // Render
      virtual void beforeRender(){

      }
      virtual void render(){

      }
      virtual void afterRender(){

      }
    };

    struct ListenerInfo {

    };

    typedef Lambda<void(const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component)> ListenerLambda;    

  }
#endif