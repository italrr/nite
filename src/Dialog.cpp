#include "Dialog.hpp"
#include <memory>

struct Token {
	int type;
	nite::Color color;
	int position;
	String value;
	bool found;
	Token(){
		found = false;
	}
};

namespace TokenType {
	enum TokenType : int {
		UNDEFINED,
		COLOR_SET,
		COLOR_RESET
	};
	static int type(const String &name){
		if(name == "cs"){
			return COLOR_SET;
		}else
		if(name == "cr"){
			return COLOR_RESET;
		}else{
			return UNDEFINED;
		}
	}
}

static Token fetchToken(int stPos, const String &input){
	String found = "";
	Token token;
	for(int i = stPos; i < input.size(); ++i){
		if(i < input.size()-1 && input[i] == '$' && input[i+1] == '['){
			int end = input.find("]", i+1);
			if(end != std::string::npos){
				found = input.substr(i+2, end-i-2);
				token.position = end;
				break;
			}
		}
	}
	if(found.size() > 0){
		int colon = found.find(":");
		if(colon != std::string::npos){
			String first = found.substr(0, colon);
			String value = found.substr(colon+1, found.length()-colon);
			token.type = TokenType::type(first);
			token.value = value;
		}else{
			token.type = TokenType::type(found);
		}
		token.found = true;
	}
	return token;
}

bool Game::DialogHook::isReady(){
    return done && lines.size() == currentDiag || currentDiag == 0 && lines.size() == 0;
}

bool Game::DialogHook::canCont(){
    return isReady() && this->proceed;
}

bool Game::DialogHook::canNext(){
    return currentDiag < lines.size() && currenText.size() >= targetText.size() || isReady();
}

void Game::DialogHook::cont(){
    if(this->ready && !this->autocont){
        this->proceed = true;
        onCont();
        if(isReady()){
            nite::print("[debug] dialog called callback");
            onEndCallback();        
        }        
    }        
}

void Game::DialogHook::setAutoCont(UInt64 timeout){
    autocont = true;
    autoContTimeout = timeout;
    autoContTick = nite::getTicks();
}

UInt64 Game::DialogHook::getLastReady(){
    return nite::getTicks()-lastReady;
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
    lastReady = nite::getTicks();
    done = false;
    ready = false;
    proceed = true;
    targetText = "";
    currenText = "";
    currentDiag = 0;
    currentChar = 0;
    autocont = false;
    lastChar = nite::getTicks();
    lines.clear();
    delay = false;
    autoContTick = nite::getTicks();
    onEndCallback = [](){

    };
    onReset();
}

void Game::DialogHook::setImmediateText(const String &text){
    targetText = text;
    currenText = text;
    onUpdateText();
}

void Game::DialogHook::step(){

    if(!proceed && ready && autocont && nite::getTicks()-autoContTick > autoContTimeout){
        proceed = true;
        autocont = false;
        onCont();
        if(isReady()){
            nite::print("[debug] dialog[autocont] called callback");
            onEndCallback();        
        }         
    }
    if(nite::getTicks()-lastChar < 20 || delay || currenText.size() >= targetText.size()){
        if(delay && nite::getTicks()-delayTick > delayTimeout){
            delay = false;
        }
        if(currenText.size() >= targetText.size()){
            this->ready = true;
            if(currentDiag >= lines.size()){
                if(!done){
                    lastReady = nite::getTicks();
                }
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
    
    if(currentChar < targetText.length()-1 && targetText[currentChar] == '$' && targetText[currentChar+1] == '['){
        auto token = fetchToken(currentChar, targetText);
        if(token.found){
            currentChar = token.position;
        }
    }  

    ++currentChar;  

    // in-text delay
    if(targetText[currentChar] == '@' && currentChar < targetText.length()-1){
        for(int i = currentChar+1; i < targetText.size(); ++i){
            if(targetText[i] == '!'){
                int start = currentChar;
                int nchars =  i-currentChar + 1;
                String token = targetText.substr(currentChar, nchars);
                if(token.size()-2 > 0){
                    token.erase(0, 1);
                    token.erase(token.size()-1, 1);
                    if(nite::isNumber(token)){
                        delay = true;
                        delayTimeout = nite::toInt(token);
                        delayTick = nite::getTicks();
                        targetText.erase(start, nchars);
                    }
                }
                break;
            }
        }
    }

    currenText = targetText.substr(0, currentChar);

    onUpdateText();
    
    lastChar = nite::getTicks();
}

Game::DialogBox::DialogBox(){
    reset();
    useStBColor = false;

    this->onCont = [&](){
        if(isShowing() && isReady()){
            if(this->dialogBox.get() != NULL){
                this->dialogBox->setVisible(false);
            }
            reset();
        }        
    };

    this->dialogBox = std::make_shared<Game::UIDialogBox>(Game::UIDialogBox());

    onReset = [&](){
        dialogBox->setText("");  
        dialogBox->setVisible(false);
    };

    onUpdateText = [&](){
        dialogBox->setText(currenText);      
    };

    onNextLine = [&](const Shared<DialogLine> &line){
        dialogBox->setTitle(line->emitter.size() > 0, line->emitter, line->color);
    };

}

void Game::DialogBox::start(const nite::Vec2 &pos, int width, int nlines, bool useTitle){
    if(done){
        return;
    }

    theme = std::make_shared<Game::UITheming>(Game::UITheming());
    theme->load("data/ui/dialogbox_theme.json");
    dialogBox->theme = theme;
    dialogBox->onCreate();
    targetText = "";


    dialogBox->setVisible(true);
    dialogBox->setSize(nite::Vec2(nite::getWidth() * 0.75f, dialogBox->theme->bigFont.getHeight() * 4.0f));
    dialogBox->position = nite::Vec2(nite::getWidth() * 0.5f - dialogBox->size.x * 0.5f, nite::getHeight() - dialogBox->size.y - 16);

    nite::print("[debug] started dialog");
}

bool Game::DialogBox::isShowing(){
    return dialogBox->visible;
}

void Game::DialogBox::update(){
    step();
    dialogBox->showArrow = canNext() && !autocont;
    dialogBox->step();
}

void Game::DialogBox::render(){
    nite::setDepth(nite::DepthTop);
    nite::setRenderTarget(nite::RenderTargetUI);  
    dialogBox->render();
}