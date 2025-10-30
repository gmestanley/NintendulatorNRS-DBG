#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "Tape.h"
#include "MapperInterface.h"

namespace Controllers {
#include <pshpack1.h>
struct ExpPort_SuborKeyboard_State {
	unsigned char Bits;
	unsigned char Row;
	unsigned char Keys[13];
};
#include <poppack.h>

int ExpPort_SuborKeyboard::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);
	writeWord(len);
	writeArray(State, len);
	return clen;
}

int ExpPort_SuborKeyboard::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;
	readWord(len);
	readArraySkip(State, len, sizeof(*State));
	return clen;
}

void ExpPort_SuborKeyboard::Frame (unsigned char mode) {
	int row, col;
	if (mode & MOV_PLAY)
		for (row = 0; row < 13; row++) State->Keys[row] = MovData[row];
	else {
		const int keymap[13][8] = {
		//	 1		2		4		8		10		20		40		80
			{DIK_4,		DIK_G,		DIK_F,		DIK_C,		DIK_F2,		DIK_E,		DIK_5,		DIK_V},          // 0
			{DIK_2,		DIK_D,		DIK_S,		DIK_END,	DIK_F1,		DIK_W,		DIK_3,		DIK_X},          // 1
			{DIK_INSERT,	DIK_BACKSPACE,	DIK_PGDN,	DIK_RIGHT,	DIK_F8,		DIK_PGUP,	DIK_DELETE,	DIK_HOME},       // 2
			{DIK_9,		DIK_I,		DIK_L,		DIK_COMMA,	DIK_F5,		DIK_O,		DIK_0,		DIK_PERIOD},     // 3
			{DIK_RBRACKET,	DIK_RETURN,	DIK_UP,		DIK_LEFT,	DIK_F7,		DIK_LBRACKET,	DIK_BACKSLASH,	DIK_DOWN},       // 4
			{DIK_Q,		DIK_CAPITAL,	DIK_Z,		DIK_TAB,	DIK_ESCAPE,	DIK_A,		DIK_1,		DIK_LCONTROL},   // 5
			{DIK_7,		DIK_Y,		DIK_K,		DIK_M,		DIK_F4,		DIK_U,		DIK_8,		DIK_J},          // 6
			{DIK_MINUS,	DIK_SEMICOLON,	DIK_APOSTROPHE,	DIK_SLASH,	DIK_F6,		DIK_P,		DIK_EQUALS,	DIK_LSHIFT},     // 7
			{DIK_T,		DIK_H,		DIK_N,		DIK_SPACE,	DIK_F3,		DIK_R,		DIK_6,		DIK_B},          // 8
			{0,		0,		0,		0,		0 /*EXT*/,	0,		0,		0}, 		 // 9
			{DIK_LMENU,	DIK_NUMPAD4,	DIK_NUMPAD7,	DIK_F11,	DIK_F12,	DIK_NUMPAD1,	DIK_NUMPAD2,	DIK_NUMPAD8},    // 10
			{DIK_SUBTRACT,	DIK_ADD,  	DIK_MULTIPLY,	DIK_NUMPAD9,	DIK_F10,	DIK_NUMPAD5,	DIK_DIVIDE,	DIK_NUMLOCK},    // 11
			{DIK_GRAVE,	DIK_NUMPAD6,	DIK_PAUSE,	DIK_SPACE,	DIK_F9,		DIK_NUMPAD3,	DIK_DECIMAL,	DIK_NUMPAD0}	 // 12
		};
		for (row =0; row <13; row++) {
			State->Keys[row] = 0;
			for (col = 0; col < 8; col++) if (KeyState[keymap[row][col]] &0x80 && (keymap[row][col] !=DIK_CAPITAL || scrollLock)) State->Keys[row] |= 1 <<col; // Recognize CAPS only when Scrll Lock is active
		}
		// Right SHIFT and CTRL like left SHIFT and CTRL
		if (KeyState[DIK_RSHIFT] &0x80) State->Keys[7] |= 0x80;
		if (KeyState[DIK_RCONTROL] &0x80) State->Keys[5] |= 0x80;
		
		// Denote the presence of extended rows only if one of their keys has been pressed
		if (State->Keys[10] || State->Keys[11] || State->Keys[12]) State->Keys[9] |=0x10; // Extended key pressed
	}
	if (mode & MOV_RECORD)
		for (row = 0; row < 13; row++) MovData[row] = State->Keys[row];
}

unsigned char ExpPort_SuborKeyboard::Read1 (void) {
	unsigned char result = 0;
	result |= Tape::Input() <<2 &0x04;
	if (State->Bits &4 && Port1->Type != STD_SUBORMOUSE) result |= 0x01; // Printer ready
	return result;
}

unsigned char ExpPort_SuborKeyboard::Read2 (void) {
	unsigned char result = 0;
	if (State->Bits &4 && State->Row < 13) result |=~State->Keys[State->Row] >>(State->Bits &2? 4: 0) <<1 &0x1E;
	return result;
}

unsigned char ExpPort_SuborKeyboard::ReadIOP (uint8_t) {
	return 0;
}

void ExpPort_SuborKeyboard::Write (unsigned char val) {
	//Tape::Output(!!(val &2));
	if (val &4) {
		if (val &1)
			State->Row = 0;
		else
		if (~val &2 && State->Bits &2)
			State->Row++;
	}
	State->Bits =val;
}

void ExpPort_SuborKeyboard::Config (HWND hWnd) {
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}

void ExpPort_SuborKeyboard::SetMasks (void) {
	MaskKeyboard = TRUE;
}

ExpPort_SuborKeyboard::~ExpPort_SuborKeyboard (void) {
	delete State;
	delete[] MovData;
}

ExpPort_SuborKeyboard::ExpPort_SuborKeyboard (DWORD *buttons) {
	Type = EXP_SUBORKEYBOARD;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_SuborKeyboard_State;
	MovLen = 13;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Row = 0;
	State->Bits = 0;
	ZeroMemory(State->Keys, sizeof(State->Keys));
}

void ExpPort_SuborKeyboard::CPUCycle (void) { 
	Tape::CPUCycle();
}

} // namespace Controllers