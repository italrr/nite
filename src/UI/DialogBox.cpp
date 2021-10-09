#include "DialogBox.hpp"
#include "../Entity.hpp"

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

void Game::UIDialogBox::setTitle(bool v, const String &title, const nite::Color &color){
    this->title = title;
    this->showTitle = v;
    this->titleColor = color;
}

Game::UIDialogBox::UIDialogBox(){
    targetText = "";
    visible = false;
    margin.set(32.0f);
    diagArrowFlip = false;
    diagArrowTick = nite::getTicks();
}

void Game::UIDialogBox::setSize(const nite::Vec2 &size){
    this->size = size;
    setText(targetText);
}

void Game::UIDialogBox::clear(){
    setText("");
}

void Game::UIDialogBox::setText(const String &text){
    nite::Vec2 left(size.x - margin.x, size.y - margin.y);
    auto _reset = [&](){
        left.x = size.x - margin.x;
    };
    targetText = "";
    float cw = 0.0f;
    auto getWordWidth = [&](const String &word){
        float _w = 0.0f;
        for(int i = 0; i < word.size(); ++i){

            if(i < word.size()-1 && word[i] == '$' && word[i+1] == '['){
                auto token = fetchToken(i, word);
                if(token.found){
                    i = token.position;
                    continue;
                }
            }


            _w += theme->bigFont.getWidth(String("")+word[i]);
        }
        return _w;
    };
    auto words = nite::split(text, ' ');
    for(int i = 0; i < words.size(); ++i){
        auto word = words[i] + " ";
        float ww = getWordWidth(word);
        if(ww > left.x){
            _reset();
            targetText += '\n';
        }
        left.x -= ww;    
        targetText += word;
    }     
}

void Game::UIDialogBox::refresh(){
   
}

void Game::UIDialogBox::onCreate(){
    if(!selArrowWhite.isLoaded()){
        selArrowWhite.load("data/texture/ui/arrow_white.png", nite::Color(1.0f, 0.0f, 0.0f, 1.0f));
    } 
    overworld = Shared<EntityOverworld>(NULL);
    setTitle(false);
}

void Game::UIDialogBox::onDestroy(){

}

void Game::UIDialogBox::step(){

}

void Game::UIDialogBox::setVisible(bool v){
    visible = v;
    setText(targetText);
}

void Game::UIDialogBox::render(){
    if(theme.get() == NULL || !visible){
        return;
    }
    // message
    nite::Vec2 p = position;

    if(overworld.get() != NULL){
        nite::Vec2 faceSize(size.y);
        nite::Vec2 pos = p;
        
        
        nite::setColor(theme->borderColor);
        theme->base.draw(pos.x + 4, pos.y + 4, faceSize.x, faceSize.y, 0.0f, 0.0f, 0.0f);
        nite::setColor(theme->backgroundColor);
        theme->base.draw(pos.x, pos.y, faceSize.x, faceSize.y, 0.0f, 0.0f, 0.0f);

     	nite::setColor(nite::Color("#503335", 1.0f));



        overworld->base.setRegion(nite::Rect(overworld->facePos.x, overworld->facePos.y, overworld->faceSize.x, overworld->faceSize.y));
        overworld->base.draw(pos.x, pos.y, faceSize.x, faceSize.y, 0.0f, 0.0f, 0.0f);
        
        p.x += faceSize.x + 8.0f;
    }

    nite::setColor(theme->borderColor);
    theme->base.draw(p.x + 4, p.y + 4, size.x, size.y, 0.0f, 0.0f, 0.0f);
    nite::setColor(theme->backgroundColor);
    theme->base.draw(p.x, p.y, size.x, size.y, 0.0f, 0.0f, 0.0f);
    nite::setColor(theme->fontColor);
    theme->bigFont.draw(targetText, p.x + margin.x * 0.5f, p.y + margin.y * 0.5f, 0.0f, 0.0f, 0.0f);

    // emitter
    if(showTitle){
        auto pmargin = nite::Vec2(0.0f, 8.0f);
        float lh = theme->smallFont.getHeight() + margin.x * 0.5f;
        float lw = theme->smallFont.getWidth(title) + margin.y * 0.5f;
        nite::Vec2 pos = p - nite::Vec2(0.0f, lh) - pmargin - nite::Vec2(overworld.get() != NULL ? size.y+8.0f : 0.0f,0.0f);
        
        nite::setColor(theme->borderColor);
        theme->base.draw(pos.x + 4, pos.y + 4, lw, lh, 0.0f, 0.0f, 0.0f);
        nite::setColor(theme->backgroundColor);
        theme->base.draw(pos.x, pos.y, lw, lh, 0.0f, 0.0f, 0.0f);

        nite::setColor(titleColor);
        theme->smallFont.draw(title, pos.x + margin.x * 0.25f, pos.y + margin.y * 0.25f, 0.0f, 0.0f, 0.0f);
    }

    // arrow
    if(nite::getTicks()-diagArrowTick > 320){
        diagArrowTick = nite::getTicks();
        diagArrowFlip = !diagArrowFlip;
    }                
    nite::lerpDiscrete(diagArrowOffset, diagArrowFlip ? 0.0f : 100.0f, diagArrowFlip ? 0.12f : 0.04f);
    nite::setDepth(nite::DepthTop);
    nite::setRenderTarget(nite::RenderTargetUI);
    auto pos = p + size * nite::Vec2(0.95f, 0.95f) + nite::Vec2(0.0f, 24.0f * (diagArrowOffset/100.0f)) - nite::Vec2(0.0f, 24.0f);
    if(showArrow){
        nite::setColor(theme->backgroundColor);
        auto arrefsha = selArrowWhite.draw(pos.x + 2, pos.y + 2, 32, 32, 0.5f, 0.5f, 0.0f);
        nite::setColor(theme->solidColor);
        auto arref = selArrowWhite.draw(pos.x, pos.y, 32, 32, 0.5f, 0.5f, 0.0f);
        if(arref != NULL){
            arref->smooth = true;
        } 
        if(arrefsha != NULL){
            arrefsha->smooth = true;
        }                     
    }           
    
        
    
}