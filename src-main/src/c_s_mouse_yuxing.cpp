#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "GFX.h"
#include <queue>
#include <stdio.h>
#include "MapperInterface.h"

namespace Controllers {
#include <pshpack1.h>
struct StdPort_YuxingMouse_State {
	unsigned char port;
	std::queue<bool> bits;
	signed short xDelta, yDelta;
	signed short yMov, xMov;
	unsigned char buttons;
};
#include <poppack.h>

int	StdPort_YuxingMouse::Save (FILE *out) {
	int clen = 0;

	uint8_t len = 0;
	std::queue<bool> queue = State->bits;
	len = queue.size() >255? 255: 0;
	writeByte(len);
	while (len--) {
		writeByte(queue.front() *1);
		queue.pop();
	}
	writeByte(State->port);
	writeWord(State->xDelta);
	writeWord(State->yDelta);
	writeWord(State->xMov);
	writeWord(State->yMov);
	writeByte(State->buttons);
	return clen;
}

int	StdPort_YuxingMouse::Load (FILE *in, int version_id) {
	int clen = 0;

	uint8_t temp = 0, len = 0;
	while (!State->bits.empty()) State->bits.pop();
	readByte(len);
	while (len--) {
		readByte(temp);
		State->bits.push(!!temp);
	}
	readByte(State->port);
	readWord(State->xDelta);
	readWord(State->yDelta);
	readWord(State->xMov);
	readWord(State->yMov);
	readByte(State->buttons);
	return clen;
}

void	StdPort_YuxingMouse::Frame (unsigned char mode) {
	if (mode & MOV_PLAY) {
		State->xDelta =(signed char)MovData[0];
		State->yDelta =(signed char)MovData[1];
		State->buttons = MovData[2];
	} else {
		State->buttons =0;
		if (IsPressed(Buttons[0])) State->buttons |= 0x1;
		if (IsPressed(Buttons[1])) State->buttons |= 0x4;
		if (IsPressed(Buttons[2])) State->buttons |= 0x2;
		State->xDelta =GetDelta(Buttons[3]);
		State->yDelta =GetDelta(Buttons[4]);
	}
	if (mode & MOV_RECORD) {
		MovData[0] =(unsigned char)(State->xDelta & 0xFF);
		MovData[1] =(unsigned char)(State->yDelta & 0xFF);
		MovData[2] =(unsigned char)(State->buttons & 0x07);
	}
	State->xMov +=State->xDelta;
	State->yMov +=State->yDelta;
}

unsigned char StdPort_YuxingMouse::Read (void) {
	unsigned char result =0;
	if (State->port &4)
		result |=State->buttons || State->yDelta || State->xDelta? 1: 0;
	else
		if (State->port &1 && !State->bits.empty()) result |=(State->bits.front()*1) ^1;
	return result;
}

static void addByteToSerialQueue (std::queue<bool>& to, uint8_t what) {
	to.push(false);
	for (int i =0; i <7; i++) {
		to.push(!!(what &0x40));
		what <<=1;
	}
	to.push(true);
}

void	StdPort_YuxingMouse::Write (unsigned char val) {
	if (~val &4 && ~State->port &1 && val &1) {
		if (State->bits.empty()) {
			int amountY = State->yMov;
			int amountX = State->xMov;
			if (amountY > 127) amountY = 127;
			if (amountX > 127) amountX = 127;
			if (amountY <-127) amountY =-127;
			if (amountX <-127) amountX =-127;
			addByteToSerialQueue(State->bits, 0x40 | (State->buttons &1? 0x20: 0x00) | (State->buttons &2? 0x10: 0x00) | amountX >>4 &0x0C  | amountY >>6 &0x03);
			addByteToSerialQueue(State->bits, amountY &0x3F);
			addByteToSerialQueue(State->bits, amountX &0x3F);
			State->yMov -=amountY;
			State->xMov -=amountX;
		} else
			State->bits.pop();
	}
	State->port =val;
}

INT_PTR	CALLBACK StdPort_YuxingMouse_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[5] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4};
	const int dlgButtons[5] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4};
	StdPort *Cont;
	if (uMsg ==WM_INITDIALOG) {
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont =(StdPort *)lParam;
	} else
		Cont =(StdPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 3, 2, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL, false);
}
void	StdPort_YuxingMouse::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_SNESMOUSE), hWnd, StdPort_YuxingMouse_ConfigProc, (LPARAM)this);
}

void	StdPort_YuxingMouse::SetMasks (void) {
	MaskMouse = (((Buttons[3] >> 16) == 1) || ((Buttons[4] >> 16) == 1));
}

StdPort_YuxingMouse::~StdPort_YuxingMouse (void) {
	delete State;
	delete[] MovData;
}

StdPort_YuxingMouse::StdPort_YuxingMouse (DWORD *buttons) {
	Type =STD_YUXINGMOUSE;
	NumButtons =5;
	Buttons =buttons;
	State =new StdPort_YuxingMouse_State;
	MovLen =3;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->port =0;
	State->bits =std::queue<bool>();
	State->xDelta =0;
	State->yDelta =0;
	State->xMov =0;
	State->yMov =0;
	State->buttons =0;
}

void StdPort_YuxingMouse::CPUCycle (void) { 
}

} // namespace Controllers