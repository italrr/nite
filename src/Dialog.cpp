#include "Dialog.hpp"
#include <memory>


bool Game::DialogHook::isReady(){
    return done && lines.size() == currentDiag || currentDiag == 0 && lines.size() == 0;
}

bool Game::DialogHook::canCont(){
    return isReady() && this->proceed;
}

void Game::DialogHook::cont(){
    if(this->ready){
        this->proceed = true;
        onCont();
    }        
}

Game::DialogHook::DialogHook(){
    this->onCont = [](){

    };
    this->onReset = [](){

    };
    this->onUpdateText = [](){

    };
    this->onNextLine = [](const Shared<DialogLine> &line){

    };        
    reset();
}

void Game::DialogHook::add(const String &emt, const String &text, const nite::Color &color){
    auto line = std::make_shared<Game::DialogLine>(Game::DialogLine(emt, text, color));
    this->lines.push_back(line);
}

void Game::DialogHook::start(){
    targetText = "";
}

void Game::DialogHook::reset(){
    done = false;
    ready = false;
    proceed = true;
    targetText = "";
    currenText = "";
    currentDiag = 0;
    currentChar = 0;
    lastChar = nite::getTicks();
    lines.clear();
    onReset();
}

void Game::DialogHook::step(){
    if(nite::getTicks()-lastChar < 20 || currenText.size() >= targetText.size()){
        if(currenText.size() >= targetText.size()){
            this->ready = true;
            if(currentDiag >= lines.size()){
                done = true;
            }else
            if(proceed){
                currentChar = 0;
                currenText = "";
                proceed = false;
                ready = false;
                targetText = lines[currentDiag]->message;

                onNextLine(lines[currentDiag]);

                ++currentDiag;
            }
        }
        return;
    }
    
    ++currentChar;

    currenText = targetText.substr(0, currentChar);

    onUpdateText();
    
    lastChar = nite::getTicks();
}

Game::DialogBox::DialogBox(){
    reset();
    useStBColor = false;

    this->onCont = [&](){
        if(isShowing() && isReady()){
            nite::print("[debug] finish dialog");
            reset();
        }        
    };

    this->onReset = [&](){
        if(textWin.get() != NULL){
            std::dynamic_pointer_cast<nite::WindowUI>(textWin)->close();
            textWin = nullptr;
        }
        if(emtWin.get() != NULL){
            std::dynamic_pointer_cast<nite::WindowUI>(emtWin)->close();
            emtWin = nullptr;
        } 
    };

    this->onUpdateText = [&](){
        updWinValue(textWin, currenText);
    };

    this->onNextLine = [&](const Shared<DialogLine> &line){
        updTextColor(emtWin, line->color);
        updWinValue(emtWin, line->emitter);

        updWinBorderColor(textWin, line->color);
        updWinBorderColor(emtWin, line->color);
    };

}

void Game::DialogBox::setStaticBorderColor(bool v, const nite::Color &color){
    useStBColor = v;
    borderColor = color;
    if(v){
        updWinBorderColor(textWin, borderColor);
        updWinBorderColor(emtWin, borderColor);
    }
}

void Game::DialogBox::setBgColor(const nite::Color &color){
    if(textWin.get() != NULL && textWin->type == "window"){
        auto cwin = std::dynamic_pointer_cast<nite::WindowUI>(textWin);
        cwin->setBackgroundColor(color);
    }else{
        nite::print("DialogBox::updWinValue: window doesn't exist");
    }   
}

void Game::DialogBox::start(const nite::Vec2 &pos, int width, int nlines, bool useTitle){
    if(done){
        return;
    }

    auto actpos = pos;

    static const float borderWdith = 5.0f;

    if(!font.isLoaded()){
        font.load("data/font/BebasNeueRegular-X34j2.otf", 36, 2);
    }
    if(!subFont.isLoaded()){
        subFont.load("data/font/BebasNeueRegular-X34j2.otf", 20, 1.0f);
    }
    if(!empty.isLoaded()){
        empty.load("data/texture/empty.png");
    }


    if(textWin.get() != NULL){
        std::dynamic_pointer_cast<nite::WindowUI>(textWin)->close();
    }
    if(emtWin.get() != NULL){
        std::dynamic_pointer_cast<nite::WindowUI>(emtWin)->close();
    }

    textWin = nite::UI::build("data/ui/dialog_window.json");
    emtWin = nite::UI::build("data/ui/dialog_window.json");

    auto twin = std::dynamic_pointer_cast<nite::WindowUI>(textWin);
    auto emwin = std::dynamic_pointer_cast<nite::WindowUI>(emtWin);

    float bwidth = 4;
    float padd = 4;
    float embwidth = 3;
    float empadd = 3;
    float h = font.getHeight() * nlines + bwidth * 2 + padd * 2;
    float emh = subFont.getHeight() + embwidth * 2 + empadd * 2;

    float digBarOffset = useTitle ? 44.0f : 0.0f;


    if(actpos.x == 0.0f){
        actpos.x = nite::getWidth() * 0.5f - (width + padd * 2) * 0.5f;
        // actpos.x = 8.0f;
        // dialogPos.x = actpos.x - 8.0f;
    }

    if(actpos.y == 0.0f){
        actpos.y = nite::getHeight() - (h  + 8.0f + digBarOffset);
        dialogPos.y = actpos.y - 8.0f;
    }

    twin->setBorderThickness(borderWdith);
    twin->setPosition(nite::Vec2(0.0f, digBarOffset) + actpos);
    twin->setSize(nite::Vec2(width + padd * 2, h));
    twin->removeCornerPattern();
    twin->setShadow(true);
    if(twin->children.size() > 0 && twin->children[0]->type == "text"){
        auto text = std::dynamic_pointer_cast<nite::TextUI>(twin->children[0]);
        text->setPadding(nite::Vec2(padd * 2 + bwidth * 2, padd * 2));
        text->setFont(font);
    }else{
        nite::print("DialogBox::start: missing text child");
    }


    emwin->setBorderThickness(borderWdith);
    emwin->setPosition(actpos);
    emwin->setShadow(true);
    emwin->setSize(nite::Vec2(width * 0.33f, emh));
    emwin->removeCornerPattern();
    emwin->setVisible(useTitle);
    if(emwin->children.size() > 0 && emwin->children[0]->type == "text"){
        auto text = std::dynamic_pointer_cast<nite::TextUI>(emwin->children[0]);
        text->setPadding(nite::Vec2(empadd * 2 + embwidth * 2, empadd * 2));
        text->setFont(subFont);
    }else{
        nite::print("DialogBox::start: missing text child");
    }    
    
    updWinValue(textWin, "");
    updWinValue(emtWin, "Tester");
    targetText = "";

    nite::print("[debug] started dialog");
}

void Game::DialogBox::updWinValue(Shared<nite::BaseUIComponent> &win, const String &newval){
    if(win.get() != NULL && win->children.size() > 0){
        if(win->children[0]->type == "text"){
            auto txt = std::dynamic_pointer_cast<nite::TextUI>(win->children[0]);
            txt->setText(newval);
        }else{
            nite::print("DialogBox::updWinValue: no text was found on the window");
        }
    }else{
        nite::print("DialogBox::updWinValue: window doesn't exist");
    }
}

void Game::DialogBox::updTextColor(Shared<nite::BaseUIComponent> &win, const nite::Color &color){
    if(win.get() != NULL && win->children.size() > 0){
        if(win->children[0]->type == "text"){
            auto txt = std::dynamic_pointer_cast<nite::TextUI>(win->children[0]);
            txt->setFontColor(color);
        }else{
            nite::print("DialogBox::updWinValue: no text was found on the window");
        }
    }else{
        nite::print("DialogBox::updWinValue: window doesn't exist");
    }
}

void Game::DialogBox::updWinBorderColor(Shared<nite::BaseUIComponent> &win, const nite::Color &color){
    if(win.get() != NULL && win->type == "window"){
        auto cwin = std::dynamic_pointer_cast<nite::WindowUI>(win);
        cwin->setBorderColor(useStBColor ? borderColor : color);
    }else{
        nite::print("DialogBox::updWinValue: window doesn't exist");
    }    
}

bool Game::DialogBox::isShowing(){
    return textWin.get() != nullptr && textWin->isVisible();
}

void Game::DialogBox::render(){
	// nite::setRenderTarget(nite::RenderTargetUI);
	// nite::setDepth(nite::DepthBottom);
    // nite::setColor(0.0f, 0.0f, 0.0f, 0.55f);
    // empty.draw(dialogPos.x, dialogPos.y, nite::getWidth(), nite::getHeight()-dialogPos.y, 0.0f, 0.0f, 0.0f);
}