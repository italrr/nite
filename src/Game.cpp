#include "Engine/Graphics.hpp"
#include "Engine/Input.hpp"
#include "Engine/View.hpp"
#include "Engine/UI/UI.hpp"

int main(int argc, char* argv[]){
	Vector<String> params;
	for(int i = 0; i < argc; ++i){
		params.push_back(String(argv[i]));
	}
	nite::setParameters(params);

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