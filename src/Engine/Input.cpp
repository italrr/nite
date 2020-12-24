#include <SDL2/SDL.h>
#include <map>
#include "Tools/Tools.hpp"
#include "Input.hpp"
#include "Console.hpp"
//#include "UI/UI.hpp"
#include "Network.hpp"
#include "Graphics.hpp"

//#include "nite.h"
//#include "View.h"

using namespace nite;

static Uint8 *St;
static nite::Vec2 lastMousePosition;
static nite::Vec2 mouseCurrentPosition;

int nite::mouseX(){
	return mouseCurrentPosition.x;
}

int nite::mouseY(){
	return mouseCurrentPosition.y;
}

nite::Vec2 nite::mousePosition(){
	return mouseCurrentPosition;
}

int nite::mouseXAdj(unsigned target){
	return mousePositionAdj(target).x;
}

int nite::mouseYAdj(unsigned target){
	return mousePositionAdj(target).y;
}

nite::Vec2 nite::mousePositionAdj(unsigned target){
	nite::Vec2 mcent = nite::mousePosition() - nite::getSize() * nite::Vec2(0.5f);
	nite::Vec2 scent = nite::getSize() * nite::Vec2(0.5f);
	float ang = nite::arctan(mcent.y, mcent.x);
	float mod = nite::sqrt(nite::pow(mcent.x, 2.0f) + nite::pow(mcent.y, 2.0f)) / nite::getZoom(nite::RenderTargetGame);
	return nite::Vec2(nite::cos(ang) * mod, nite::sin(ang) * mod) + nite::getSize() * nite::Vec2(0.5f);
}

float nite::mouseSpeed(){
	nite::Vec2 currentMousePosition = nite::mousePosition();
	return nite::distance(lastMousePosition, currentMousePosition);
}

int nite::translateKey(String key){
	key = nite::toUpper(key);
	if(key == "A"){return nite::keyA;} else if(key == "B"){return nite::keyB;} else if(key == "C"){return nite::keyC;} else if(key == "D"){return nite::keyD;} else if(key == "E"){return nite::keyE;} else
	if(key == "F"){return nite::keyF;} else if(key == "G"){return nite::keyG;} else if(key == "H"){return nite::keyH;} else if(key == "I"){return nite::keyI;} else if(key == "J"){return nite::keyJ;} else
	if(key == "K"){return nite::keyK;} else if(key == "L"){return nite::keyL;} else if(key == "M"){return nite::keyM;} else if(key == "N"){return nite::keyN;} else if(key == "O"){return nite::keyO;} else
	if(key == "P"){return nite::keyP;} else if(key == "Q"){return nite::keyQ;} else if(key == "R"){return nite::keyR;} else if(key == "S"){return nite::keyS;} else if(key == "T"){return nite::keyT;} else
	if(key == "U"){return nite::keyU;} else if(key == "V"){return nite::keyV;} else if(key == "W"){return nite::keyW;} else if(key == "X"){return nite::keyX;} else if(key == "Y"){return nite::keyY;} else
	if(key == "Z"){return nite::keyZ;} else if(key == "0"){return nite::key0;} else if(key == "1"){return nite::key1;} else if(key == "2"){return nite::key2;} else if(key == "3"){return nite::key3;} else
	if(key == "4"){return nite::key4;} else if(key == "5"){return nite::key5;} else if(key == "6"){return nite::key6;} else if(key == "7"){return nite::key7;} else if(key == "8"){return nite::key8;} else
	if(key == "9"){return nite::key9;} else if(key == "ESCAPE"){return nite::keyESCAPE;} else if(key == "LCONTROL"){return nite::keyLCONTROL;} else if(key == "LSHIFT"){return nite::keyLSHIFT;} else
	if(key == "LALT"){return nite::keyLALT;} else if(key == "LSYSTEM"){return nite::keyLSYSTEM;} else if(key == "RCONTROL"){return nite::keyRCONTROL;} else if(key == "RSHIFT"){return nite::keyRSHIFT;} else
	if(key == "RALT"){return nite::keyRALT;} else if(key == "RSYSTEM"){return nite::keyRSYSTEM;} else if(key == "MENU"){return nite::keyMENU;} else if(key == "LBRACKET"){return nite::keyLBRACKET;} else
	if(key == "RBRACKET"){return nite::keyRBRACKET;} else if(key == "SEMICOLON"){return nite::keySEMICOLON;} else if(key == "COMMA"){return nite::keyCOMMA;} else if(key == "PERIOD"){return nite::keyPERIOD;} else
	if(key == "QUOTE"){return nite::keyQUOTE;} else if(key == "SLASH"){return nite::keySLASH;} else if(key == "BACKSLASH"){return nite::keyBACKSLASH;} else if(key == "TILDE"){return nite::keyTILDE;} else
	if(key == "EQUAL"){return nite::keyEQUAL;} else if(key == "DASH"){return nite::keyDASH;} else if(key == "SPACE"){return nite::keySPACE;} else if(key == "ENTER"){return nite::keyENTER;} else
	if(key == "BACK"){return nite::keyBACK;} else if(key == "PAGEUP"){return nite::keyPAGEUP;} else if(key == "PAGEDOWN"){return nite::keyPAGEDOWN;} else if(key == "END"){return nite::keyEND;} else
	if(key == "HOME"){return nite::keyHOME;} else if(key == "INSERT"){return nite::keyINSERT;} else if(key == "DELETE"){return nite::keyDELETE;} else if(key == "ADD"){return nite::keyADD;} else
	if(key == "SUBTRACT"){return nite::keySUBTRACT;} else if(key == "MULTIPLY"){return nite::keyMULTIPLY;} else if(key == "DIVIDE"){return nite::keyDIVIDE;} else if(key == "LEFT"){return nite::keyLEFT;} else
	if(key == "RIGHT"){return nite::keyRIGHT;} else if(key == "UP"){return nite::keyUP;} else if(key == "DOWN"){return nite::keyDOWN;} else if(key == "NUMPAD0"){return nite::keyNUMPAD0;} else
	if(key == "NUMPAD1"){return nite::keyNUMPAD1;} else if(key == "NUMPAD2"){return nite::keyNUMPAD2;} else if(key == "NUMPAD3"){return nite::keyNUMPAD3;} else if(key == "NUMPAD4"){return nite::keyNUMPAD4;} else
	if(key == "NUMPAD5"){return nite::keyNUMPAD5;} else if(key == "NUMPAD6"){return nite::keyNUMPAD6;} else if(key == "NUMPAD7"){return nite::keyNUMPAD7;} else if(key == "NUMPAD8"){return nite::keyNUMPAD8;} else
	if(key == "NUMPAD9"){return nite::keyNUMPAD9;} else if(key == "F1"){return nite::keyF1;} else if(key == "F2"){return nite::keyF2;} else if(key == "F3"){return nite::keyF3;} else if(key == "F4"){return nite::keyF4;} else
	if(key == "F5"){return nite::keyF5;} else if(key == "F6"){return nite::keyF6;} else if(key == "F7"){return nite::keyF7;} else if(key == "F8"){return nite::keyF8;} else if(key == "F9"){return nite::keyF9;} else
	if(key == "F10"){return nite::keyF10;} else if(key == "F11"){return nite::keyF11;} else if(key == "F12"){return nite::keyF12;} else if(key == "F14"){return nite::keyF14;} else if(key == "F14"){return nite::keyF14;} else
	if(key == "F15"){return nite::keyF15;} else if(key == "PAUSE"){return nite::keyPAUSE;}else
	if(key == "M1"){ return nite::butLEFT; } else if(key == "M2"){ return nite::butRIGHT; } else
	return -1;
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
	return false;
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
			return butLEFT - 201;
		case SDL_BUTTON_MIDDLE:
			return butMIDDLE - 201;
		case SDL_BUTTON_RIGHT:
			return butRIGHT - 201;
		default:
			return niteButtonsn-1;
		break;
	}
}


bool nite::mouseCheck(unsigned Button){
	Button -= 201;
	return (Button >= niteButtonsn ? 0 : buttonState[Button]);
}


bool nite::mousePressed(unsigned Button){
	Button -= 201;
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
	Button -= 201;
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
					auto *ins = Game::getGameCoreInstance();
					ins->end();
				// Game::exit();
			break;
		}
	}
	if(nite::keyboardPressed(nite::keyTILDE)){
		if(nite::Console::isOpen()){
			nite::Console::close();
		}else{
			nite::Console::open();
		}
	}	
	nite::Console::update();
	// nite::UI::update();	
	nite::AsyncTask::update();
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
