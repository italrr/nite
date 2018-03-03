#include <SDL2/SDL.h>
#include <map>
#include "Tools/Tools.hpp"
#include "Input.hpp"

//#include "nite.h"
//#include "View.h"

using namespace nite;

static Uint8 *St;
static nite::Vec2 lastMousePosition;
static nite::Vec2 mouseCurrentPosition;
/*
================
Get currently mouse's x
================
*/
int nite::mouseX(){
	return mouseCurrentPosition.x;
}

/*
================
Get currently mouse's y
================
*/
int nite::mouseY(){
	return mouseCurrentPosition.y;
}

/*
================
Get currently mouse's x and y
================
*/
nite::Vec2 nite::mousePosition(){
	return mouseCurrentPosition;
}

/*
================
Get mouse's speed
================
*/
float nite::mouseSpeed(){
	nite::Vec2 currentMousePosition = nite::mousePosition();
	return nite::distance(lastMousePosition, currentMousePosition);
}

static const int niteKeys[] = {
	SDLK_a,SDLK_b,SDLK_c,SDLK_d,SDLK_e,SDLK_f,SDLK_g,SDLK_h,SDLK_i,
	SDLK_j,SDLK_k,SDLK_l,SDLK_m,SDLK_n,SDLK_o,SDLK_p,SDLK_q,SDLK_r,
	SDLK_s,SDLK_t,SDLK_u,SDLK_v,SDLK_w,SDLK_x,SDLK_y,SDLK_z,SDLK_0,
	SDLK_1,SDLK_2,SDLK_3,SDLK_4,SDLK_5,SDLK_6,SDLK_7,SDLK_8,SDLK_9,
	SDLK_ESCAPE,SDLK_LCTRL,SDLK_LSHIFT,SDLK_LALT,SDLK_LGUI,SDLK_RCTRL,
	SDLK_RSHIFT,SDLK_RALT,SDLK_RGUI,SDLK_MENU,SDLK_LEFTBRACKET,
	SDLK_RIGHTBRACKET,SDLK_SEMICOLON,SDLK_COMMA,SDLK_PERIOD,SDLK_QUOTE,
	SDLK_SLASH,SDLK_BACKSLASH,SDLK_BACKQUOTE,SDLK_EQUALS,SDLK_SPACE,SDLK_SPACE,
	SDLK_RETURN,SDLK_BACKSPACE,SDLK_TAB,SDLK_PAGEUP,SDLK_PAGEDOWN,SDLK_END,
	SDLK_HOME,SDLK_INSERT,SDLK_DELETE,SDLK_PLUS,SDLK_MINUS,SDLK_ASTERISK,
	SDLK_SLASH,SDLK_LEFT,SDLK_RIGHT,SDLK_UP,SDLK_DOWN,SDLK_KP_0,SDLK_KP_1,
	SDLK_KP_2,SDLK_KP_3,SDLK_KP_4,SDLK_KP_5,SDLK_KP_6,SDLK_KP_7,SDLK_KP_8,
	SDLK_KP_9,SDLK_F1,SDLK_F2,SDLK_F3,SDLK_F4,SDLK_F5,SDLK_F6,SDLK_F7,
	SDLK_F8,SDLK_F9,SDLK_F10,SDLK_F11,SDLK_F12,SDLK_F13,SDLK_F14,SDLK_F15,
	SDLK_PAUSE
};

bool nite::isMouseInRegion(float x, float y,float x2, float y2, unsigned target){
	// unsigned r = target;
	// if (nite::mouseX()+nite::getViewX(r)>=x and nite::mouseX()+nite::getViewX(r)<=x2 and
		// nite::mouseY()+nite::getViewY(r)>=y and nite::mouseY()+nite::getViewY(r)<=y2){
		// return 1;
	// }
	// return 0;
}

bool nite::isMouseInRegion(const nite::Vec2 &P, const nite::Vec2 &Q, unsigned target){
	return isMouseInRegion(P.x, P.y, Q.x, Q.y, target);
}

bool nite::isMouseInRegion(float x, float y,float x2, float y2){
	if (nite::mouseX()>=x and nite::mouseX()<=x2 and
		nite::mouseY()>=y and nite::mouseY()<=y2){
		return 1;
	}
	return 0;
}

bool nite::isMouseInRegion(const nite::Vec2 &P, const nite::Vec2 &Q){
	return isMouseInRegion(P.x, P.y, Q.x, Q.y);
}

static const unsigned niteKeysn = sizeof(niteKeys)/sizeof(int);;

static Dict<unsigned, unsigned> SDLKeys;

void nite::inputInit(){
	for (unsigned i=0; i<niteKeysn; i++){
		SDLKeys[niteKeys[i]] = i;
	}
}

static unsigned getKey(unsigned key){
	return SDLKeys[key];
}

static bool KeysState [niteKeysn];
static bool KeysCheck [niteKeysn];
static bool KeysStatePressed [niteKeysn];
static bool isKeysStatePressed [niteKeysn];
static bool KeysStateReleased [niteKeysn];
static bool isKeysStateReleased [niteKeysn];

bool nite::keyboardCheck(unsigned key){
	SDL_Scancode k = SDL_GetScancodeFromKey(niteKeys[key]);
	static const Uint8 *state = SDL_GetKeyboardState(NULL);
	return (key >= niteKeysn-1 ? 0 : state[k]);
}

bool nite::keyboardPress(unsigned key){
	return (key >= niteKeysn-1 ? 0 : KeysCheck[key]);
}

bool nite::keyboardPressOnce(unsigned key){
	if (key >= niteKeysn-1) return 0;
	bool b = KeysCheck[key];
	KeysState[key] = 0;
	KeysStatePressed[key] = 0;
	isKeysStatePressed[key] = 0;
	KeysCheck[key] = 0;
	return b;
}

bool nite::keyboardPressed(unsigned key){
	if (key >= niteKeysn){
		return 0;
	}

	if (isKeysStatePressed[key]){
		return 1;
	}

	if (KeysStatePressed[key] and KeysState[key]){
		return 0;
	}

	if (KeysStatePressed[key]){
		KeysStatePressed[key] = 0;
		return 0;
	}

	if (!KeysStatePressed[key] and KeysState[key]){
		isKeysStatePressed[key] = 1;
		KeysStatePressed[key] = 1;
		return 1;
	}

	return 0;
}


bool nite::keyboardReleased(unsigned key){
	if (key >= niteKeysn){
		return 0;
	}

	if (isKeysStateReleased[key]){
		return 1;
	}

	if (KeysState[key]){
		KeysStateReleased[key] = 1;
		return 0;
	}

	if (!KeysState[key] and KeysStateReleased[key]){
		KeysStateReleased[key] = 0;
		isKeysStateReleased[key] = 1;
		return 1;
	}

	return 0;
}

#define niteButtonsn 4

static bool buttonState[niteButtonsn];
static bool buttonCheck[niteButtonsn];
static bool buttonStatePressed[niteButtonsn];
static bool isbuttonStatePressed[niteButtonsn];
static bool buttonStateReleased[niteButtonsn];
static bool isbuttonStateReleased[niteButtonsn];

static unsigned getButton(unsigned Button){
	switch(Button){
		case SDL_BUTTON_LEFT:
			return butLEFT;
		case SDL_BUTTON_MIDDLE:
			return butMIDDLE;
		case SDL_BUTTON_RIGHT:
			return butRIGHT;
		default:
			return niteButtonsn-1;
		break;
	}
}


bool nite::mouseCheck(unsigned Button){
	return (Button >= niteButtonsn ? 0 : buttonState[Button]);
}


bool nite::mousePressed(unsigned Button){
	if (Button >= niteButtonsn){
		return 0;
	}

	if (isbuttonStatePressed[Button]){
		return 1;
	}

	if (buttonStatePressed[Button] and buttonState[Button]){
		return 0;
	}

	if (buttonStatePressed[Button]){
		buttonStatePressed[Button] = 0;
		return 0;
	}

	if (!buttonStatePressed[Button] and buttonState[Button]){
		isbuttonStatePressed[Button] = 1;
		buttonStatePressed[Button] = 1;
		return 1;
	}

	return 0;
}

bool nite::mouseReleased(unsigned Button){
	if (Button >= niteKeysn){
		return 0;
	}

	if (isbuttonStateReleased[Button]){
		return 1;
	}

	if (buttonState[Button]){
		buttonStateReleased[Button] = 1;
		return 0;
	}

	if (!buttonState[Button] and buttonStateReleased[Button]){
		buttonStateReleased[Button] = 0;
		isbuttonStateReleased[Button] = 1;
		return 1;
	}

	return 0;
}

#include "../Game.hpp"

/*
================
Refresh keys and buttons status (Should be executed every step in the main loop)
================
*/
static SDL_Event Events;
static unsigned lastSpeedUpdate = nite::getTocks();
void nite::inputUpdate(){
	int x;
	int y;
	SDL_GetMouseState(&x, &y);
	mouseCurrentPosition.set(x, y);
	if(nite::getTocks()-lastSpeedUpdate > 1){
		lastMousePosition = nite::mousePosition();
		lastSpeedUpdate = nite::getTocks();
	}
	unsigned key;
	for(unsigned i=0; i<niteKeysn; i++){
		KeysCheck[i] = 0;
		isKeysStatePressed[i] = 0;
		isKeysStateReleased[i] = 0;
	}
	for(unsigned i=0; i<niteButtonsn; i++){
		isbuttonStatePressed [i] = 0;
		isbuttonStateReleased [i] = 0;
	}
	while(SDL_PollEvent(&Events)){
		switch(Events.type){
			case SDL_KEYUP:
				KeysState[getKey(Events.key.keysym.sym)] = 0;
			break;
			case SDL_KEYDOWN:
				KeysCheck[getKey(Events.key.keysym.sym)] = 1;
				KeysState[getKey(Events.key.keysym.sym)] = 1;
			break;
			case SDL_MOUSEBUTTONDOWN:
				buttonState[getButton(Events.button.button)] = 1;
			break;
			case SDL_MOUSEBUTTONUP:
				buttonState[getButton(Events.button.button)] = 0;
			break;
			case SDL_QUIT:
					auto *ins = Game::getInstance();
					ins->end();
				// Game::exit();
			break;
		}
	}
	return;
}

String nite::keyboardGetText(){
	String Char = "";
	bool Upper = 0;
	if (nite::keyboardCheck(keyRSHIFT) or nite::keyboardCheck(keyLSHIFT)){
		Upper = 1;
	}
	if (nite::keyboardPressOnce(keySUBTRACT)){
		Char = ( Upper ? "_":"-" );
	}else
	if (nite::keyboardPressOnce(keyEQUAL)){
		Char = ( Upper ? "+":"=" );
	}else
	if (nite::keyboardPressOnce(keyCOMMA)){
		Char = ( Upper ? "<":"," );
	}else
	if (nite::keyboardPressOnce(keyPERIOD)){
		Char = ( Upper ? ">":"." );
	}else
	if (nite::keyboardPressOnce(keyDIVIDE)){
		Char = ( Upper ? "?":"/" );
	}else
	if (nite::keyboardPressOnce(keyQUOTE)){
		Char = ( Upper ? "\"":"'" );
	}else
	if (nite::keyboardPressOnce(keySEMICOLON)){
		Char = ( Upper ? ":":";" );
	}else
	if (nite::keyboardPressOnce(keyLBRACKET)){
		Char = ( Upper ? "{":"[" );
	}else
	if (nite::keyboardPressOnce(keyRBRACKET)){
		Char = ( Upper ? "}":"]" );
	}else
	if (nite::keyboardPressOnce(keyBACKSLASH)){
		Char = ( Upper ? "|":"\\" );
	}else
	if (nite::keyboardPressOnce(key0)){
		Char = ( Upper ? ")":"0" );
	}else
	if (nite::keyboardPressOnce(key1)){
		Char = ( Upper ? "!":"1" );
	}else
	if (nite::keyboardPressOnce(key2)){
		Char = ( Upper ? "@":"2" );
	}else
	if (nite::keyboardPressOnce(key3)){
		Char = ( Upper ? "#":"3" );
	}else
	if (nite::keyboardPressOnce(key4)){
		Char = ( Upper ? "$":"4" );
	}else
	if (nite::keyboardPressOnce(key5)){
		Char = ( Upper ? "%":"5" );
	}else
	if (nite::keyboardPressOnce(key6)){
		Char = ( Upper ? "^":"6" );
	}else
	if (nite::keyboardPressOnce(key7)){
		Char = ( Upper ? "&":"7" );
	}else
	if (nite::keyboardPressOnce(key8)){
		Char = ( Upper ? "*":"8" );
	}else
	if (nite::keyboardPressOnce(key9)){
		Char = ( Upper ? "(":"9" );
	}else
	if (nite::keyboardPressOnce(keyA)){
		Char = "a";
	}else
	if (nite::keyboardPressOnce(keyB)){
		Char = "b";
	}else
	if (nite::keyboardPressOnce(keyC)){
		Char = "c";
	}else
	if (nite::keyboardPressOnce(keyD)){
		Char = "d";
	}else
	if (nite::keyboardPressOnce(keyE)){
		Char = "e";
	}else
	if (nite::keyboardPressOnce(keyF)){
		Char = "f";
	}else
	if (nite::keyboardPressOnce(keyG)){
		Char = "g";
	}else
	if (nite::keyboardPressOnce(keyH)){
		Char = "h";
	}else
	if (nite::keyboardPressOnce(keyI)){
		Char = "i";
	}else
	if (nite::keyboardPressOnce(keyJ)){
		Char = "j";
	}else
	if (nite::keyboardPressOnce(keyK)){
		Char = "k";
	}else
	if (nite::keyboardPressOnce(keyL)){
		Char = "l";
	}else
	if (nite::keyboardPressOnce(keyM)){
		Char = "m";
	}else
	if (nite::keyboardPressOnce(keyN)){
		Char = "n";
	}else
	if (nite::keyboardPressOnce(keyO)){
		Char = "o";
	}else
	if (nite::keyboardPressOnce(keyP)){
		Char = "p";
	}else
	if (nite::keyboardPressOnce(keyQ)){
		Char = "q";
	}else
	if (nite::keyboardPressOnce(keyR)){
		Char = "r";
	}else
	if (nite::keyboardPressOnce(keyS)){
		Char = "s";
	}else
	if (nite::keyboardPressOnce(keyT)){
		Char = "t";
	}else
	if (nite::keyboardPressOnce(keyU)){
		Char = "u";
	}else
	if (nite::keyboardPressOnce(keyV)){
		Char = "v";
	}else
	if (nite::keyboardPressOnce(keyW)){
		Char = "w";
	}else
	if (nite::keyboardPressOnce(keyX)){
		Char = "x";
	}else
	if (nite::keyboardPressOnce(keyY)){
		Char = "y";
	}else
	if (nite::keyboardPressOnce(keyZ)){
		Char = "z";
	}else
	if (nite::keyboardPressOnce(keySPACE)){
		Char = " ";
	}
	return (Upper ? nite::toUpper(Char) : Char);
}
