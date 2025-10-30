#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers {
#include <pshpack1.h>
struct ExpPort_JissenMahjong_State {
	unsigned int Bits;
	unsigned char Sel;
	unsigned int NewBits;
};
#include <poppack.h>
int	ExpPort_JissenMahjong::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_JissenMahjong::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_JissenMahjong::Frame (unsigned char mode) {
	int i;
	if (mode & MOV_PLAY)
		State->NewBits = MovData[0] | (MovData[1] << 8);
	else {
		State->NewBits = 0;
		for (i = 0; i <22; i++) {
			if (IsPressed(Buttons[i])) State->NewBits |= 1 << i;
		}
	}
	if (mode & MOV_RECORD) {
		MovData[0] = (unsigned char)(State->NewBits & 0xFF);
		MovData[1] = (unsigned char)(State->NewBits >> 8);
	}
}
unsigned char	ExpPort_JissenMahjong::Read1 (void) {
	return 0;
}
unsigned char	ExpPort_JissenMahjong::Read2 (void) {
	unsigned char result = 0;
	result =State->Bits >>6 &2;
	State->Bits <<=1;
	return result;
}

unsigned char	ExpPort_JissenMahjong::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_JissenMahjong::Write (unsigned char Val) {
	if (State->Sel &1 && ~Val &1) switch(State->Sel >>1 &3) {
		case 0: State->Bits =State->NewBits &0xFF | State->NewBits >>8 &0x3F; break;
		case 1: State->Bits =State->NewBits >>8 &0x3F; break;
		case 2: State->Bits =State->NewBits &0xFF; break;
		case 3: State->Bits =State->NewBits >>14 &0x7F; break;
	}
	State->Sel = Val &7;
}

INT_PTR	CALLBACK	ExpPort_JissenMahjong_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[1] = {IDC_CONT_D0};
	const int dlgButtons[22] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4,IDC_CONT_K5,IDC_CONT_K6,IDC_CONT_K7,IDC_CONT_K8,IDC_CONT_K9,IDC_CONT_K10,IDC_CONT_K11,IDC_CONT_K12,IDC_CONT_K13,IDC_CONT_K14,IDC_CONT_K15,IDC_CONT_K16,IDC_CONT_K17,IDC_CONT_K18,IDC_CONT_K19,IDC_CONT_K20,IDC_CONT_K21};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG) {
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	} else
		Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 22, 0, dlgLists, dlgButtons, Cont? Cont->Buttons: NULL, true);
}

void	ExpPort_JissenMahjong::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_JISSEN_MAHJONG), hWnd, ExpPort_JissenMahjong_ConfigProc, (LPARAM)this);
}

void	ExpPort_JissenMahjong::SetMasks (void) {
}

ExpPort_JissenMahjong::~ExpPort_JissenMahjong (void) {
	delete State;
	delete[] MovData;
}
ExpPort_JissenMahjong::ExpPort_JissenMahjong (DWORD *buttons) {
	Type = EXP_JISSENMAHJONG;
	NumButtons = 22;
	Buttons = buttons;
	State = new ExpPort_JissenMahjong_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->Sel = 0;
	State->NewBits = 0;
}
void ExpPort_JissenMahjong::CPUCycle (void) { }
} // namespace Controllers