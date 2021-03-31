#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"

#include "GameType.hpp"
#include "Board.hpp"


int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);

	nite::graphicsInit();
	nite::inputInit();


	auto gType = loadGameType("data/games/traditional.json");
	Board board;
	board.init(nite::Color(205.0f / 255.0f, 230.0f / 255.0f, 208.0f / 255.0f, 1.0f), 1080, 1080, gType);

	// nite::Font font("data/font/Saniretro.ttf", 24, 1.0f);
	// nite::Texture empty("data/texture/empty.png");
	// static const String text = "LORE ";
	// float maxWidth = 78;
	while(nite::isRunning()){	
		nite::viewUpdate();
		nite::inputUpdate();	
		board.draw(0, 0);
		// nite::setColor(0.1f, 0.1f, 0.1f, 1.0f);
		// empty.draw(nite::getWidth()*0.5, nite::getHeight()*0.5, maxWidth, font.getHeight(text), 0.5f, 0.5f, 0.0f);
		// nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
		// nite::print(font.getWidth(text));
		// auto obj = font.draw(text,nite::getWidth()*0.5f, nite::getHeight()*0.5f, 0.5f, 0.5f, 0.0f);
		// obj->horSpace = maxWidth;
		// obj->autobreak = true;
		// obj->align = nite::TextAlign::CENTER;
		nite::graphicsRender();
	}
	nite::graphicsEnd();


	return 0;
}