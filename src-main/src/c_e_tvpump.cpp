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
struct ExpPort_TVPump_State {
	unsigned char buttons;
};
#include <poppack.h>
int	ExpPort_TVPump::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}

int	ExpPort_TVPump::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}

void	ExpPort_TVPump::Frame (unsigned char mode) {
	if (mode & MOV_PLAY) {
		State->buttons =MovData[0];
	} else {
		State->buttons = IsPressed(Buttons[0])*1 + IsPressed(Buttons[1])*2 + IsPressed(Buttons[2])*4 + IsPressed(Buttons[3])*8 + IsPressed(Buttons[4])*16 + IsPressed(Buttons[5])*32;
	}
	if (mode & MOV_RECORD) {
		MovData[0] = (unsigned char) State->buttons;
	}
}
unsigned char	ExpPort_TVPump::Read1 (void) {
	return 0;
}
unsigned char	ExpPort_TVPump::Read2 (void) {
	return 0;
}

unsigned char	ExpPort_TVPump::ReadIOP (uint8_t which) {
	return which ==4? ~State->buttons: ~0;
}

void	ExpPort_TVPump::Write (unsigned char) {
}

INT_PTR	CALLBACK	ExpPort_TVPump_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[6] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4,IDC_CONT_D5};
	const int dlgButtons[6] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4,IDC_CONT_K5};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 6, 0, dlgLists, dlgButtons, Cont? Cont->Buttons: NULL, false);
}
void	ExpPort_TVPump::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXPPORT_TVPUMP), hWnd, ExpPort_TVPump_ConfigProc, (LPARAM)this);
}

void	ExpPort_TVPump::SetMasks (void) {
	MaskMouse = ((Buttons[0] >>16) ==1);
}

ExpPort_TVPump::~ExpPort_TVPump (void) {
	delete State;
	delete[] MovData;
}

ExpPort_TVPump::ExpPort_TVPump (DWORD *buttons) {
	Type = EXP_TVPUMP;
	NumButtons =6;
	Buttons =buttons;
	State =new ExpPort_TVPump_State;
	MovLen =1;
	MovData =new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->buttons =0;
}
void ExpPort_TVPump::CPUCycle (void) { }
} // namespace Controllers