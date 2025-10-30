#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers {
#include <pshpack1.h>
struct StdPort_SuborMouse_State {
	unsigned long Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	int Xmov, Ymov;
	int Xdelta, Ydelta;
	unsigned char Buttons;
	unsigned char ByteNum;
};
#include <poppack.h>

int	StdPort_SuborMouse::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}

int	StdPort_SuborMouse::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

void	StdPort_SuborMouse::Frame (unsigned char mode) { 
	if (mode & MOV_PLAY) {
		State->Xdelta = (signed char)MovData[0];
		State->Ydelta = (signed char)MovData[1];
		State->Buttons = MovData[2];
	} else {
		State->Buttons = 0;
		if (IsPressed(Buttons[0])) State->Buttons |= 0x2;
		if (IsPressed(Buttons[2])) State->Buttons |= 0x1;
		State->Xdelta = GetDelta(Buttons[3]);
		State->Ydelta = GetDelta(Buttons[4]);
		MaskMouse =CursorOnOutput;
	}
	if (mode & MOV_RECORD) {
		MovData[0] = (unsigned char)(State->Xdelta & 0xFF);
		MovData[1] = (unsigned char)(State->Ydelta & 0xFF);
		MovData[2] = (unsigned char)(State->Buttons & 0x03);
	}
	State->Xmov +=State->Xdelta;
	State->Ymov +=State->Ydelta;
}

unsigned char	StdPort_SuborMouse::Read (void) {
	unsigned char result = 0x00;
	if (State->BitPtr <24) result |= State->Bits << State->BitPtr++ >>23 &0x01;
	return result;
}

void	StdPort_SuborMouse::Write (unsigned char val) {
	if (State->Strobe && ~val &1) {
		int amountY = State->Ymov;
		int amountX = State->Xmov;
		if (amountY <-127) amountY =-127;
		if (amountX <-127) amountX =-127;
		if (amountY > 127) amountY = 127;
		if (amountX > 127) amountX = 127;
		State->Bits = !!(amountY < 0)*0x080000 | !!(amountY > 0)*0x040000 | !!(amountX < 0)*0x020000 | !!(amountX > 0)*0x010000 | // direction
			      !!(abs(amountX) > 0 || abs(amountY) > 0 || !!State->Buttons) *0x200000 | // mouse features
			      State->Buttons <<22 | // buttons
			      abs(amountY) | abs(amountX) <<8 | // amount
			      !!State->Buttons *0x200000;
		State->BitPtr = 0;
		State->Ymov -= amountY;
		State->Xmov -= amountX;
	}
	State->Strobe =!!(val &1 && ~val &2 && ~val &4);
}

INT_PTR	CALLBACK	StdPort_SuborMouse_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[5] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4};
	const int dlgButtons[5] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4};
	StdPort *Cont;
	if (uMsg == WM_INITDIALOG) {
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (StdPort *)lParam;
	} else	
		Cont = (StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 3, 2, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL, false);
}

void	StdPort_SuborMouse::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_SNESMOUSE), hWnd, StdPort_SuborMouse_ConfigProc, (LPARAM)this);
}

void	StdPort_SuborMouse::SetMasks (void) {
	MaskMouse = ((Buttons[3] >> 16) == 1) || ((Buttons[4] >> 16) == 1);
}

StdPort_SuborMouse::~StdPort_SuborMouse (void) {
	delete State;
	delete[] MovData;
}

StdPort_SuborMouse::StdPort_SuborMouse (DWORD *buttons) {
	Type =STD_SUBORMOUSE;
	NumButtons =5;
	Buttons =buttons;
	State =new StdPort_SuborMouse_State;
	MovLen =3;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->Xmov = 0;
	State->Ymov = 0;
	State->Xdelta = 0;
	State->Ydelta = 0;
	State->Buttons = 0;
	State->ByteNum = 0;
}

void StdPort_SuborMouse::CPUCycle (void) { }
} // namespace Controllers