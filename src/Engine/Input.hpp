#ifndef NITE_INPUT_HPP
	#define NITE_INPUT_HPP

	#include "Types.hpp"

	namespace nite {
		void inputInit();
		void inputUpdate();

		/* Keyboard */
		String keyboardGetText();
		bool keyboardReleased(unsigned key);
		bool keyboardPressed(unsigned key);
		bool keyboardPress(unsigned key);
		bool keyboardPressOnce(unsigned key);
		bool keyboardCheck(unsigned key);

		/* Mouse */
		bool mousePressed(unsigned Button);
		bool mouseReleased(unsigned Button);
		bool mouseCheck(unsigned Button);
		float mouseSpeed();
		int mouseX();
		int mouseY();
		nite::Vec2 mousePosition();
		bool isMouseInRegion(float x, float y, float x2, float y2, unsigned target);
		bool isMouseInRegion(const nite::Vec2 &P, const nite::Vec2 &Q, unsigned target);
		bool isMouseInRegion(float x, float y, float x2, float y2);
		bool isMouseInRegion(const nite::Vec2 &P,  const nite::Vec2 &Q);
		int translateKey(String key);

		const unsigned keyA 				= 0;
		const unsigned keyB 				= 1;
		const unsigned keyC 				= 2;
		const unsigned keyD 				= 3;
		const unsigned keyE 				= 4;
		const unsigned keyF 				= 5;
		const unsigned keyG 				= 6;
		const unsigned keyH 				= 7;
		const unsigned keyI 				= 8;
		const unsigned keyJ 				= 9;
		const unsigned keyK 				= 10;
		const unsigned keyL 				= 11;
		const unsigned keyM 				= 12;
		const unsigned keyN 				= 13;
		const unsigned keyO 				= 14;
		const unsigned keyP 				= 15;
		const unsigned keyQ 				= 16;
		const unsigned keyR 				= 17;
		const unsigned keyS 				= 18;
		const unsigned keyT 				= 19;
		const unsigned keyU 				= 20;
		const unsigned keyV 				= 21;
		const unsigned keyW 				= 22;
		const unsigned keyX 				= 23;
		const unsigned keyY 				= 24;
		const unsigned keyZ 				= 25;
		const unsigned key0 				= 26;
		const unsigned key1 				= 27;
		const unsigned key2 				= 28;
		const unsigned key3 				= 29;
		const unsigned key4 				= 30;
		const unsigned key5 				= 31;
		const unsigned key6 				= 32;
		const unsigned key7 				= 33;
		const unsigned key8 				= 34;
		const unsigned key9 				= 35;
		const unsigned keyESCAPE 			= 36;
		const unsigned keyLCONTROL 			= 37;
		const unsigned keyLSHIFT 			= 38;
		const unsigned keyLALT 				= 39;
		const unsigned keyLSYSTEM 			= 40;
		const unsigned keyRCONTROL 			= 41;
		const unsigned keyRSHIFT 			= 42;
		const unsigned keyRALT 				= 43;
		const unsigned keyRSYSTEM 			= 44;
		const unsigned keyMENU 				= 45;
		const unsigned keyLBRACKET 			= 46;
		const unsigned keyRBRACKET 			= 47;
		const unsigned keySEMICOLON 		= 48;
		const unsigned keyCOMMA 			= 49;
		const unsigned keyPERIOD 			= 50;
		const unsigned keyQUOTE 			= 51;
		const unsigned keySLASH 			= 52;
		const unsigned keyBACKSLASH 		= 53;
		const unsigned keyTILDE 			= 54; // ~
		const unsigned keyEQUAL 			= 55;
		const unsigned keyDASH 				= 56;
		const unsigned keySPACE 			= 57;
		const unsigned keyENTER 			= 58;
		const unsigned keyBACK 				= 59;
		const unsigned keyTAB 				= 60;
		const unsigned keyPAGEUP 			= 61;
		const unsigned keyPAGEDOWN 			= 62;
		const unsigned keyEND 				= 63;
		const unsigned keyHOME 				= 64;
		const unsigned keyINSERT 			= 65;
		const unsigned keyDELETE 			= 66;
		const unsigned keyADD 				= 67;
		const unsigned keySUBTRACT 			= 68;
		const unsigned keyMULTIPLY 			= 69;
		const unsigned keyDIVIDE 			= 70;
		const unsigned keyLEFT 				= 71;
		const unsigned keyRIGHT 			= 72;
		const unsigned keyUP 				= 73;
		const unsigned keyDOWN 				= 74;
		const unsigned keyNUMPAD0 			= 75;
		const unsigned keyNUMPAD1 			= 76;
		const unsigned keyNUMPAD2 			= 77;
		const unsigned keyNUMPAD3 			= 78;
		const unsigned keyNUMPAD4 			= 79;
		const unsigned keyNUMPAD5 			= 80;
		const unsigned keyNUMPAD6 			= 81;
		const unsigned keyNUMPAD7 			= 82;
		const unsigned keyNUMPAD8 			= 83;
		const unsigned keyNUMPAD9 			= 84;
		const unsigned keyF1 				= 85;
		const unsigned keyF2 				= 86;
		const unsigned keyF3 				= 87;
		const unsigned keyF4 				= 88;
		const unsigned keyF5 				= 89;
		const unsigned keyF6 				= 90;
		const unsigned keyF7 				= 91;
		const unsigned keyF8 				= 92;
		const unsigned keyF9 				= 93;
		const unsigned keyF10 				= 94;
		const unsigned keyF11 				= 95;
		const unsigned keyF12 				= 96;
		const unsigned keyF13 				= 97;
		const unsigned keyF14 				= 98;
		const unsigned keyF15 				= 99;
		const unsigned keyPAUSE				= 100;

		const unsigned butLEFT				= 201;
		const unsigned butMIDDLE			= 202;
		const unsigned butRIGHT				= 203;
	}
#endif
