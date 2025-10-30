#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "GFX.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_ABLPinball_State
{
	signed short count;
	signed short plunger;
	unsigned char buttons;
	int plungerResetCount;
};
#include <poppack.h>
int	ExpPort_ABLPinball::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}

int	ExpPort_ABLPinball::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

void	ExpPort_ABLPinball::Frame (unsigned char mode) {
	if (mode & MOV_PLAY) {
		State->buttons =MovData[0];
		State->plunger =MovData[1];
	} else {
		State->buttons =(IsPressed(Buttons[0])? 0x10: 0x00) | // Select
		                (IsPressed(Buttons[1])? 0x01: 0x00) | // Start
		                (IsPressed(Buttons[2])? 0x02: 0x00) | // Left
		                (IsPressed(Buttons[3])? 0x04: 0x00) | // Right
		                (IsPressed(Buttons[4])? 0x08: 0x00);  // Nudge
		State->plunger +=GetDelta(Buttons[5]) /5;
		if (State->plunger <1)  State->plunger =1;
		if (State->plunger >20) State->plunger =20;
	}
	if (mode & MOV_RECORD) {
		MovData[0] = State->buttons;
		MovData[1] = (unsigned char) State->plunger;
	}
}
unsigned char	ExpPort_ABLPinball::Read1 (void) {
	if (State->plunger ==0) State->plunger++;
	int result =State->count <0? 1: State->count <State->plunger? 0: 1;
	result |=State->buttons >>3 &0x02; // Select
	if (++State->count >=State->plunger) State->count =-1;
	return result;
}
unsigned char	ExpPort_ABLPinball::Read2 (void) {
	return State->buttons &0x07; // Start, Left, Right
}

unsigned char	ExpPort_ABLPinball::ReadIOP (uint8_t which) {
	return which ==3? State->buttons &0x08: 0;
}

void	ExpPort_ABLPinball::Write (unsigned char) {
}

INT_PTR	CALLBACK	ExpPort_ABLPinball_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[6] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4,IDC_CONT_D5};
	const int dlgButtons[6] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4,IDC_CONT_K5};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 5, 1, dlgLists, dlgButtons, Cont? Cont->Buttons: NULL, false);
}
void	ExpPort_ABLPinball::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXPPORT_ABLPINBALL), hWnd, ExpPort_ABLPinball_ConfigProc, (LPARAM)this);
}

void	ExpPort_ABLPinball::SetMasks (void) {
	MaskMouse = ((Buttons[5] >>16) ==1);
}

ExpPort_ABLPinball::~ExpPort_ABLPinball (void) {
	delete State;
	delete[] MovData;
}

ExpPort_ABLPinball::ExpPort_ABLPinball (DWORD *buttons) {
	Type = EXP_ABLPINBALL;
	NumButtons =6;
	Buttons =buttons;
	State =new ExpPort_ABLPinball_State;
	MovLen =2;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->buttons =0;
	State->plunger =1;
	State->count =-1;
	State->plungerResetCount =0;
}
void ExpPort_ABLPinball::CPUCycle (void) { 
	if (++State->plungerResetCount ==1789773/4) { // Slowly move the plunger back up, so it does not become stuck the next time it is needed
		State->plungerResetCount =0;
		if (State->plunger >0) State->plunger--;
	}
}
} // namespace Controllers