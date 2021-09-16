#include "DialogBox.hpp"


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



Game::UIDialogBox::UIDialogBox(){
    targetText = "";
    visible = false;
    margin.set(16.0f);
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


            _w += theme->regularFont.getWidth(String("")+word[i]);
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
    nite::Vec2 p = position;
    nite::setColor(theme->borderColor);
    theme->base.draw(position.x + 4, position.y + 4, size.x, size.y, 0.0f, 0.0f, 0.0f);
    nite::setColor(theme->backgroundColor);
    theme->base.draw(position.x, position.y, size.x, size.y, 0.0f, 0.0f, 0.0f);
    nite::setColor(theme->fontColor);
    theme->regularFont.draw(targetText, p.x + margin.x * 0.5f, p.y + margin.y * 0.5f, 0.0f, 0.0f, 0.0f);
}