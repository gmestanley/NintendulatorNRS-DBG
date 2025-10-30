#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"
#include "GFX.h"
#include "CPU.h"
#include "PS2DevicePort.h"

namespace Controllers {
#include <pshpack1.h>
struct StdPort_PS2Mouse_State {
	int Xdelta, Ydelta;
	unsigned char Buttons;
	PS2DevicePort port;
	bool direction;
};
#include <poppack.h>
	
int	StdPort_PS2Mouse::Save (FILE *out) {	
	int clen = 0;
	unsigned short len = State->port.saveLoad(STATE_SIZE, 0, NULL);
	unsigned char *tpmi = new unsigned char[len];
	State->port.saveLoad(STATE_SAVE, 0, tpmi);
	writeWord(len);
	writeArray(tpmi, len);
	delete[] tpmi;
	
	writeLong(State->Xdelta);
	writeLong(State->Ydelta);
	writeByte(State->Buttons);
	writeBool(State->direction);
	return clen;
}

int	StdPort_PS2Mouse::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;
	readWord(len);	
	unsigned char *tpmi = new unsigned char[len];
	readArray(tpmi, len);
	if (len == State->port.saveLoad(STATE_SIZE, 0, NULL)) State->port.saveLoad(STATE_LOAD, 0, tpmi);
	delete[] tpmi;

	readLong(State->Xdelta);
	readLong(State->Ydelta);
	readByte(State->Buttons);
	readBool(State->direction);
	return clen;
}

void	StdPort_PS2Mouse::Frame (unsigned char mode) { 
	if (mode & MOV_PLAY) {
		State->Xdelta = (signed char)MovData[0];
		State->Ydelta = (signed char)MovData[1];
		State->Buttons = MovData[2];
	} else {
		State->Buttons = 0;
		if (IsPressed(Buttons[0])) State->Buttons |= 0x1;
		if (IsPressed(Buttons[1])) State->Buttons |= 0x4;
		if (IsPressed(Buttons[2])) State->Buttons |= 0x2;
		int Xdelta = GetDelta(Buttons[3]);
		int Ydelta = GetDelta(Buttons[4]);
		State->Xdelta =(Xdelta >>1) | (Xdelta &1);
		State->Ydelta =(Ydelta >>1) | (Ydelta &1);
		MaskMouse =CursorOnOutput;
	}
	if (mode & MOV_RECORD) {
		MovData[0] = (unsigned char)(State->Xdelta & 0xFF);
		MovData[1] = (unsigned char)(State->Ydelta & 0xFF);
		MovData[2] = (unsigned char)(State->Buttons & 0x07);
	}
}

unsigned char	StdPort_PS2Mouse::Read (void) {
	State->port.setHostClock(true);
	return (State->direction? !State->port.getData(): State->port.getClock()) *0x01;
}

void	StdPort_PS2Mouse::Write (unsigned char val) {
	if (RI.InputType ==INPUT_BBK_KBD_MOUSE) {
		State->direction =!(val &0x04);
		State->port.setHostData (!(val &0x01));
	} else {
		State->port.setHostClock(true);
		State->direction =!!(val &0x04);
		State->port.setHostData(!!(val &0x01));
	}
}

INT_PTR	CALLBACK	StdPort_PS2Mouse_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

void	StdPort_PS2Mouse::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_STDPORT_SNESMOUSE), hWnd, StdPort_PS2Mouse_ConfigProc, (LPARAM)this);
}

void	StdPort_PS2Mouse::SetMasks (void) {
	MaskMouse = ((Buttons[3] >> 16) == 1) || ((Buttons[4] >> 16) == 1);
}

StdPort_PS2Mouse::~StdPort_PS2Mouse (void) {
	delete State;
	delete[] MovData;
}

StdPort_PS2Mouse::StdPort_PS2Mouse (DWORD *buttons) {
	Type =STD_PS2MOUSE;
	NumButtons =5;
	Buttons =buttons;
	State =new StdPort_PS2Mouse_State;
	MovLen =3;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Xdelta = 0;
	State->Ydelta = 0;
	State->Buttons = 0;
	State->direction =false;
}

void StdPort_PS2Mouse::CPUCycle (void) { 
	State->port.cpuCycle();
	if (State->port.dataFromHost.size() >=1 && State->port.dataFromHost.front() ==0xEB) {
		State->port.dataFromHost.pop();
		while (!State->port.dataToHost.empty()) State->port.dataToHost.pop();
		State->port.dataToHost.push(0xFA);
		int Y;
		int X;
		int buttons;
		if (RI.InputType ==INPUT_BBK_KBD_MOUSE) {
			X = State->Ydelta; // Rotate 90 degrees to the right
			Y =-State->Xdelta;
			buttons =!!(State->Buttons &4)*2 +!!(State->Buttons &2)*1 +!!(State->Buttons &1)*4;
		} else
		if (RI.InputType ==INPUT_SUBOR_PS2INV_2P) {
			X =-State->Xdelta;
			Y = State->Ydelta;
			buttons =!!(State->Buttons &4)*4 +!!(State->Buttons &2)*1 +!!(State->Buttons &1)*2;
		} else {
			X =-State->Ydelta; // Rotate 90 degrees to the left
			Y = State->Xdelta;
			buttons =!!(State->Buttons &4)*4 +!!(State->Buttons &2)*1 +!!(State->Buttons &1)*2;
		}
		
		uint8_t byte1 =buttons &7 | 8 | (X <0? 0x10: 0) | (Y <0? 0x20: 0);		
		uint8_t byte2 =X;
		uint8_t byte3 =Y;
		State->port.dataToHost.push(byte1);
		State->port.dataToHost.push(byte2);
		State->port.dataToHost.push(byte3);
		//EI.DbgOut(L"Read Mouse: 4 bytes to buffer, now %d", State->port.dataToHost.size());
	} else
	if (State->port.dataFromHost.size() >=1 && State->port.dataFromHost.front() ==0xFF) {
		State->port.dataFromHost.pop();
		while (!State->port.dataToHost.empty()) State->port.dataToHost.pop();
		State->port.dataToHost.push(0xFA);
		State->port.dataToHost.push(0xAA);
		State->port.dataToHost.push(0x00);
		//EI.DbgOut(L"Reset: 3 bytes to buffer");
	} else
	if (State->port.dataFromHost.size() >=1 && State->port.dataFromHost.front() ==0xF0) { // Set Remote Mode
		State->port.dataFromHost.pop();
		while (!State->port.dataToHost.empty()) State->port.dataToHost.pop();
		State->port.dataToHost.push(0xFA);
	} else
	if (State->port.dataFromHost.size()) {
		State->port.dataFromHost.pop();
		while (!State->port.dataToHost.empty()) State->port.dataToHost.pop();
		//EI.DbgOut(L"Invalid command %02X", State->port.dataFromHost.front());
	}
}

} // namespace Controllers