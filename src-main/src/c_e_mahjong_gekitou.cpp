#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers {
#include <pshpack1.h>
struct ExpPort_MahjongGekitou_State {
	unsigned char Bits;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char NewBits;
};
#include <poppack.h>
int	ExpPort_MahjongGekitou::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_MahjongGekitou::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_MahjongGekitou::Frame (unsigned char mode) {
	if (mode & MOV_PLAY) {
		State->NewBits = MovData[0];
	} else {
		static const uint8_t buttonToRowColumn[22] ={
			{ 0x80}, // A
			{ 0x40}, // B
			{ 0x82}, // C
			{ 0x88}, // D
			{ 0x81}, // E
			{ 0x84}, // F
			{ 0xA0}, // G
			{ 0x90}, // H
			{ 0x42}, // I
			{ 0x48}, // J
			{ 0x41}, // K
			{ 0x44}, // L
			{ 0x60}, // M
			{ 0x50}, // N
			{ 0x02}, // SELECT
			{ 0x50}, // START, map to N
			{ 0x08}, // 槓
			{ 0x01}, // 碰
			{ 0x04}, // 吃
			{ 0x20}, // 立直
			{ 0x10}, // 栄
			{ 0x62} // Test mode
		};
		State->NewBits =0;
		for (int button =0; button <22; button++) if (IsPressed(Buttons[button])) State->NewBits |=buttonToRowColumn[button];
	}
	if (mode & MOV_RECORD) MovData[0] =State->NewBits;
}
unsigned char	ExpPort_MahjongGekitou::Read1 (void) {
	unsigned char result;
	if (State->Strobe) {
		State->Bits = State->NewBits;
		State->BitPtr = 0;
		result = (unsigned char)(State->Bits & 1);
	} else {
		if (State->BitPtr <8)
			result = (unsigned char)(State->Bits >> (7-State->BitPtr)) &1;
		else
			result = 1;

		if (State->BitPtr <8) State->BitPtr++;
	}
	return result;
}

unsigned char	ExpPort_MahjongGekitou::Read2 (void) {
	return 0;
}

unsigned char	ExpPort_MahjongGekitou::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_MahjongGekitou::Write (unsigned char Val) {
	if (State->Strobe || Val & 1) 	{
		State->Strobe = Val & 1;
		State->Bits = State->NewBits;
		State->BitPtr = 0;
	}
}

INT_PTR	CALLBACK	ExpPort_MahjongGekitou_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

void	ExpPort_MahjongGekitou::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_JISSEN_MAHJONG), hWnd, ExpPort_MahjongGekitou_ConfigProc, (LPARAM)this);
}

void	ExpPort_MahjongGekitou::SetMasks (void) {

}
ExpPort_MahjongGekitou::~ExpPort_MahjongGekitou (void) {
	delete State;
	delete[] MovData;
}

ExpPort_MahjongGekitou::ExpPort_MahjongGekitou (DWORD *buttons) {
	Type = EXP_MAHJONGGEKITOU;
	NumButtons = 22;
	Buttons = buttons;
	State = new ExpPort_MahjongGekitou_State;
	MovLen = 1;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->NewBits = 0;
}

void ExpPort_MahjongGekitou::CPUCycle (void) {
}
} // namespace Controllers