#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"
#include "Engine/Network.hpp"

int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);
	bool onlyClient = false;
	for(int i = 0; i < params.size(); ++i){
		if(params[i] == "cl"){
			onlyClient = true;
		}
	}

	nite::graphicsInit();
	nite::inputInit();
	
	while(nite::isRunning()){	
		nite::viewUpdate();
		nite::inputUpdate();

		nite::graphicsRender();
	}
	nite::graphicsEnd();


	return 0;
}