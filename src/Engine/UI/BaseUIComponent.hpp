#ifndef NITE_BASIC_UI_COMPONENT_HPP
  #define NITE_BASIC_UI_COMPONENT_HPP

  #include "../Graphics.hpp"
  #include "../Shader.hpp"
  #include "../Tools/Tools.hpp"
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

    static const String defaultFontUI = "data/font/monaco.ttf";
    static const float defaultFontRatio = 1.5f;
    static const float componentTranslationSpeed = 0.25f;

    struct PoliVec2 : nite::Vec2 {
        bool useAbs;
        nite::Vec2 abs;
        nite::Vec2 rel;
        PoliVec2();
        PoliVec2(float x, float y);
        PoliVec2(const String &w, const String &h);
        PoliVec2(const nite::Vec2 &size);
        void set(const String &w, const String &h);
        void set(const nite::Vec2 &size);
        void set(float x, float y);
        float x(float rw); // rw = region width
        float y(float rh);
    };

    struct BaseUIComponent {
        nite::Vec2 position;
        nite::Vec2 realPosition;
        nite::Vec2 size;
        nite::Vec2 relSize;
        bool useRelSizeX;
        bool useRelSizeY;
        nite::Vec2 margin;
        nite::Vec2 padding;
        nite::Vec2 relativePosition;
        nite::Color baseColor;
        nite::UIMaster *currentMaster;
        int zdepth;
        String literalId;
        bool fillUpType;
        String type;
        unsigned renderOnTarget;
        std::shared_ptr<nite::LayoutSystemUI> layout;
        Vector<std::shared_ptr<nite::BaseUIComponent>> children;
        BaseUIComponent *headComponent;
        float flex;
        bool solid;
        bool visible;
        int id;
        UInt64 uiCreatedTimeout;
        String componentName;
        bool uninteract;
        Dict<String, Shared<nite::BaseUIComponent>> idsLookUp;
        Dict<String, Jzon::Node> styles;
        nite::Shader shader;
        bool userShader;

        virtual void apply(const nite::Shader &shader){
            this->shader = shader;
            userShader = true;
        }

        virtual void unapply(){
            userShader = false;
        }

        // nicely optmized with pre-cached data
        Shared<nite::BaseUIComponent> getComponentById(const String &id){
            auto it = idsLookUp.find(id);
            if(it == idsLookUp.end()){
                return Shared<nite::BaseUIComponent>(NULL);
            }
            return it->second;
        }

        // complexity off the charts thou
        // must be careful to execute this one
        Shared<nite::BaseUIComponent> getComponentByType(const String &type){ // first one
            auto result = Shared<nite::BaseUIComponent>(NULL); 
            for(int i = 0; i < children.size(); ++i){
                result = children[i]->type == type ? children[i] : children[i]->getComponentByType(type);
                if(result.get() != NULL){
                    return result;
                }
            }
            return result;
        }        

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

        void setVisible(bool v){
            this->visible = v;
            recalculate();
        }

        bool isVisible(){
            return this->visible;
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

        BaseUIComponent();

        void setFlex(float flex){
            this->flex = flex;
            recalculate();
        }

        void setId(const String &id){
            this->literalId = id;
        }

        String getId(){
            return literalId;
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

        bool add(std::shared_ptr<nite::BaseUIComponent> component);

        bool remove(int id);

        void destroy();    

        void clear(){
            for(int i = 0; i < children.size(); ++i){
                auto child = children[i];
                child->onDestroy();
                child->headComponent = NULL;
            }        
            idsLookUp.clear();
            children.clear();
            styles.clear();
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