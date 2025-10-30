#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_TrifaceMahjong_State
{
	unsigned char Column;
	unsigned char Row;
	unsigned char Keys[10];
};
#include <poppack.h>
int	ExpPort_TrifaceMahjong::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_TrifaceMahjong::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_TrifaceMahjong::Frame (unsigned char mode) {
	int row;
	if (mode & MOV_PLAY) {
		for (row = 0; row <10; row++) State->Keys[row] = MovData[row];
	} else {
		static const uint8_t buttonToRowColumn[22][4] ={
			{ 0, 0x04, 0, 0x00}, // A
			{ 0, 0x02, 0, 0x00}, // B
			{ 8, 0x02, 0, 0x00}, // C
			{ 3, 0x02, 0, 0x00}, // D
			{ 6, 0x80, 0, 0x00}, // E
			{ 6, 0x04, 0, 0x00}, // F
			{ 3, 0x04, 0, 0x00}, // G
			{ 6, 0x08, 0, 0x00}, // H
			{ 8, 0x04, 0, 0x00}, // I
			{ 3, 0x20, 0, 0x00}, // J
			{ 7, 0x20, 0, 0x00}, // K
			{ 5, 0x01, 0, 0x00}, // L
			{ 8, 0x20, 0, 0x00}, // M
			{ 1, 0x04, 0, 0x00}, // N
			{ 8, 0x01, 0, 0x00}, // SELECT
			{ 1, 0x04, 0, 0x00}, // START, map to N
			{ 5, 0x20, 0, 0x00}, // 槓
			{ 8, 0x80, 0, 0x00}, // 碰
			{ 0, 0x08, 0, 0x00}, // 吃
			{ 1, 0x02, 0, 0x00}, // 立直
			{ 0, 0x20, 0, 0x00}, // 栄
			{ 5, 0x20, 8, 0x04}, // Test Mode (槓+I)
		};
		for (row =0; row <10; row++) State->Keys[row] = 0;
		for (int button =0; button <22; button++) if (IsPressed(Buttons[button])) {
			State->Keys[buttonToRowColumn[button][0]] |=buttonToRowColumn[button][1];
			State->Keys[buttonToRowColumn[button][2]] |=buttonToRowColumn[button][3];
		}
	}
	if (mode & MOV_RECORD) for (row = 0; row <10; row++)MovData[row] = State->Keys[row];
}
unsigned char	ExpPort_TrifaceMahjong::Read1 (void) {
	return 0;
}
unsigned char	ExpPort_TrifaceMahjong::Read2 (void) {
	unsigned char result = 0;
	if (State->Row <10) {
		if (State->Column)
			result = (State->Keys[State->Row] & 0xF0) >> 3;
		else	
			result = (State->Keys[State->Row] & 0x0F) << 1;
		result ^= 0x1E;
	}
	return result;
}

unsigned char	ExpPort_TrifaceMahjong::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_TrifaceMahjong::Write (unsigned char Val) {
	BOOL ResetKB = Val & 1;
	BOOL SelColumn = Val & 2;
	BOOL SelectKey = Val & 4;
	if (SelectKey) {
		if ((State->Column) && (!SelColumn))
			State->Row++;
		State->Column = SelColumn;
		if (ResetKB)
			State->Row = 0;
	}
}

INT_PTR	CALLBACK	ExpPort_TrifaceMahjong_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

void	ExpPort_TrifaceMahjong::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_JISSEN_MAHJONG), hWnd, ExpPort_TrifaceMahjong_ConfigProc, (LPARAM)this);
}

void	ExpPort_TrifaceMahjong::SetMasks (void) {
	
}
ExpPort_TrifaceMahjong::~ExpPort_TrifaceMahjong (void) {
	delete State;
	delete[] MovData;
}

ExpPort_TrifaceMahjong::ExpPort_TrifaceMahjong (DWORD *buttons) {
	Type = EXP_TRIFACEMAHJONG;
	NumButtons = 22;
	Buttons = buttons;
	State = new ExpPort_TrifaceMahjong_State;
	MovLen = 10;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Row = 0;
	State->Column = 0;
	ZeroMemory(State->Keys, sizeof(State->Keys));
}
void ExpPort_TrifaceMahjong::CPUCycle (void) { 
}
} // namespace Controllers