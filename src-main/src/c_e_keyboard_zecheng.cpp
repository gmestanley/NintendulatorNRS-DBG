#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"

namespace Controllers {
struct ExpPort_ZeChengKeyboard_State {
	DIMOUSESTATE2 mouseState;
	unsigned char bits1;
	unsigned char bits2;
	unsigned char strobe;
};

int ExpPort_ZeChengKeyboard::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);
	writeWord(len);
	writeArray(State, len);
	return clen;
}

int ExpPort_ZeChengKeyboard::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;
	readWord(len);
	readArraySkip(State, len, sizeof(*State));
	return clen;
}

void ExpPort_ZeChengKeyboard::Frame (unsigned char mode) {
	if (mode & MOV_PLAY) {
	} else {
		memcpy(&State->mouseState, &MouseState, sizeof(DIMOUSESTATE2));
	}
	if (mode & MOV_RECORD) {
	}
}
unsigned char ExpPort_ZeChengKeyboard::Read1 (void) {
	unsigned char result = State->bits1 &0x80? 0x01: 0x00;
	State->bits1 <<=1;
	return result;
}
unsigned char ExpPort_ZeChengKeyboard::Read2 (void) {
	unsigned char result = State->bits2 &0x80? 0x01: 0x00;
	State->bits2 <<=1;
	return result;
}

unsigned char ExpPort_ZeChengKeyboard::ReadIOP (uint8_t) {
	return 0;
}

void ExpPort_ZeChengKeyboard::Write (unsigned char val) {
	if (State->strobe &1 && ~val &1) {
		State->bits1 =0x00;
		State->bits2 =0x00;
		if (State->mouseState.rgbButtons[0] || State->mouseState.rgbButtons[1] || State->mouseState.lX || State->mouseState.lY) {
			State->bits2 =0x40 | (State->mouseState.rgbButtons[0]? 0x04: 0x00) | (State->mouseState.rgbButtons[1]? 0x08: 0x00);
			int deltaX =State->mouseState.lX;
			int deltaY =State->mouseState.lY;
			State->mouseState.lX =0;
			State->mouseState.lY =0;
			if (deltaX < 0) {
				State->bits2 |=0x01;
				deltaX =-deltaX;
			}
			if (deltaY < 0) {
				State->bits2 |=0x02;
				deltaY =-deltaY;
			}
			deltaX =log2(deltaX) +1;
			deltaY =log2(deltaY) +1;
			State->bits1 =deltaY | deltaX <<4;
		} else {
			State->bits1 =0x00;
			State->bits2 =0x80;
			if (KeyState[DIK_S] &0x80) State->bits1 =65;
			if (KeyState[DIK_Q] &0x80) State->bits1 =1;
			//if (KeyState[DIK_A] &0x80) State->bits1 =83;
		}
	}
	State->strobe =val &1;
}

void ExpPort_ZeChengKeyboard::Config (HWND hWnd) {
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}

void ExpPort_ZeChengKeyboard::SetMasks (void) {
	MaskKeyboard = TRUE;
	MaskMouse = TRUE;
}

ExpPort_ZeChengKeyboard::~ExpPort_ZeChengKeyboard (void) {
	delete State;
	delete[] MovData;
}

ExpPort_ZeChengKeyboard::ExpPort_ZeChengKeyboard (DWORD *buttons) {
	Type = EXP_ZECHENGKEYBOARD;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_ZeChengKeyboard_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	ZeroMemory(State, sizeof(ExpPort_ZeChengKeyboard_State));
	State->bits1 = 0;
	State->bits2 = 0;
}

void ExpPort_ZeChengKeyboard::CPUCycle (void) { 
}

} // namespace Controllers