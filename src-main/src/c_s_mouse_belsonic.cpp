#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers {
#include <pshpack1.h>
struct StdPort_BelsonicMouse_State {
	unsigned long Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	int Xdelta, Ydelta;
	int Xmov, Ymov;
	unsigned char Buttons;
	unsigned char ByteNum;
};
#include <poppack.h>

int StdPort_BelsonicMouse::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);
	writeWord(len);
	writeArray(State, len);
	return clen;
}

int StdPort_BelsonicMouse::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;
	readWord(len);
	readArraySkip(State, len, sizeof(*State));
	return clen;
}

void StdPort_BelsonicMouse::Frame (unsigned char mode) { 
	if (mode & MOV_PLAY) {
		State->Xdelta = (signed char)MovData[0];
		State->Ydelta = (signed char)MovData[1];
		State->Buttons = MovData[2];
	} else {
		State->Buttons = 0;
		if (IsPressed(Buttons[0])) State->Buttons |= 0x2;
		if (IsPressed(Buttons[1])) State->Buttons |= 0x4;
		if (IsPressed(Buttons[2])) State->Buttons |= 0x1;
		State->Xdelta = GetDelta(Buttons[3]);
		State->Ydelta = GetDelta(Buttons[4]);
		MaskMouse =CursorOnOutput;
	}
	State->Xmov +=State->Xdelta;
	State->Ymov +=State->Ydelta;
	State->ByteNum = 0;
	if (mode & MOV_RECORD) {
		MovData[0] = (unsigned char)(State->Xdelta & 0xFF);
		MovData[1] = (unsigned char)(State->Ydelta & 0xFF);
		MovData[2] = (unsigned char)(State->Buttons & 0x07);
	}
}

unsigned char StdPort_BelsonicMouse::Read (void) {
	unsigned char result = 0x00;
	if (State->BitPtr <8) result |= State->Bits << State->BitPtr++ >>7 &0x01;
	return result;
}

void StdPort_BelsonicMouse::Write (unsigned char val) {
	if (State->Strobe && ~val &1) {
		State->BitPtr = 0;
		int amountY = State->Ymov;
		int amountX = State->Xmov;
		if (amountY <-31) amountY =-31;
		if (amountX <-31) amountX =-31;
		if (amountY > 31) amountY = 31;
		if (amountX > 31) amountX = 31;
		if (amountX > 1 || amountX <-1 || amountY > 1 || amountY <-1 || State->ByteNum == 1 || State->ByteNum == 2) switch (++State->ByteNum) {
			case 1:
				State->Bits = State->ByteNum | abs(amountY) >>2 &0x04 | !!(amountY < 0)*0x08 | abs(amountX) &0x10 | !!(amountX < 0)*0x020 | State->Buttons <<6;
				break;
			case 2:
				State->Bits = State->ByteNum | abs(amountX) <<2 &0x3C;
				break;
			case 3:	State->Bits = State->ByteNum | abs(amountY) <<2 &0x3C;
				State->Ymov -= amountY;
				State->Xmov -= amountX;
				break;			
		} else {
			State->Bits =!!(amountY < 0)*0x0C | !!(amountY > 0)*0x08 | !!(amountX < 0)*0x30 | !!(amountX > 0)*0x20 | State->Buttons <<6;
			State->Ymov -= amountY;
			State->Xmov -= amountX;
		}
	}
	State->Strobe = !!(val &1 && ~val &2 && ~val &4);
}

INT_PTR	CALLBACK StdPort_BelsonicMouse_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[5] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4};
	const int dlgButtons[5] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4};
	StdPort *Cont;
	if (uMsg == WM_INITDIALOG) {
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (StdPort *)lParam;
	} else	
		Cont = (StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 3, 2, dlgLists, dlgButtons, Cont? Cont->Buttons: NULL, false);
}

void StdPort_BelsonicMouse::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_SNESMOUSE), hWnd, StdPort_BelsonicMouse_ConfigProc, (LPARAM) this);
}

void StdPort_BelsonicMouse::SetMasks (void) {
	MaskMouse = ((Buttons[3] >>16) == 1) || ((Buttons[4] >>16) == 1);
}

StdPort_BelsonicMouse::~StdPort_BelsonicMouse (void) {
	delete State;
	delete[] MovData;
}

StdPort_BelsonicMouse::StdPort_BelsonicMouse (DWORD *buttons) {
	Type =STD_BELSONICMOUSE;
	NumButtons = 5;
	Buttons = buttons;
	State =new StdPort_BelsonicMouse_State;
	MovLen = 3;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->Xdelta = 0;
	State->Ydelta = 0;
	State->Xmov = 0;
	State->Ymov = 0;
	State->Buttons = 0;
	State->ByteNum = 0;
}

void StdPort_BelsonicMouse::CPUCycle (void) { }
} // namespace Controllers
