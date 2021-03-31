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

	while(nite::isRunning()){	
		nite::viewUpdate();
		nite::inputUpdate();	
		board.update();
		board.draw(0, 0);
		nite::graphicsRender();
	}
	nite::graphicsEnd();


	return 0;
}