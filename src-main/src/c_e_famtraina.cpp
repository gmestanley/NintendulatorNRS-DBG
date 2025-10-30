#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_FamTrainerA_State
{
	unsigned short Bits;
	unsigned char Sel;
	unsigned short NewBits;
};
#include <poppack.h>
int	ExpPort_FamTrainerA::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_FamTrainerA::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_FamTrainerA::Frame (unsigned char mode)
{
	int i;
	if (mode & MOV_PLAY)
		State->NewBits = MovData[0] | (MovData[1] << 8);
	else
	{
		State->NewBits = 0;
		for (i = 0; i < 12; i++)
		{
			if (IsPressed(Buttons[i]))
				State->NewBits |= 1 << i;
		}
	}
	if (mode & MOV_RECORD)
	{
		MovData[0] = (unsigned char)(State->NewBits & 0xFF);
		MovData[1] = (unsigned char)(State->NewBits >> 8);
	}
}
unsigned char	ExpPort_FamTrainerA::Read1 (void)
{
	return 0;
}
unsigned char	ExpPort_FamTrainerA::Read2 (void)
{
	unsigned char result = 0;
	if (State->Sel & 0x1)
		result = (unsigned char)(State->Bits >> 8) & 0xF;
	else if (State->Sel & 0x2)
		result = (unsigned char)(State->Bits >> 4) & 0xF;
	else if (State->Sel & 0x4)
		result = (unsigned char)(State->Bits >> 0) & 0xF;
	return (result ^ 0xF) << 1;
}

unsigned char	ExpPort_FamTrainerA::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_FamTrainerA::Write (unsigned char Val)
{
	State->Bits = State->NewBits;
	State->Sel = ~Val & 7;
}
INT_PTR	CALLBACK	ExpPort_FamTrainerA_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const int dlgLists[12] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3,IDC_CONT_D4,IDC_CONT_D5,IDC_CONT_D6,IDC_CONT_D7,IDC_CONT_D8,IDC_CONT_D9,IDC_CONT_D10,IDC_CONT_D11};
	const int dlgButtons[12] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3,IDC_CONT_K4,IDC_CONT_K5,IDC_CONT_K6,IDC_CONT_K7,IDC_CONT_K8,IDC_CONT_K9,IDC_CONT_K10,IDC_CONT_K11};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	}
	else	Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 12, 0, dlgLists, dlgButtons, Cont ? Cont->Buttons : NULL, true);
}
void	ExpPort_FamTrainerA::Config (HWND hWnd)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FAMILYTRAINER_A), hWnd, ExpPort_FamTrainerA_ConfigProc, (LPARAM)this);
}
void	ExpPort_FamTrainerA::SetMasks (void)
{
}
ExpPort_FamTrainerA::~ExpPort_FamTrainerA (void)
{
	delete State;
	delete[] MovData;
}
ExpPort_FamTrainerA::ExpPort_FamTrainerA (DWORD *buttons)
{
	Type = EXP_FAMTRAINERA;
	NumButtons = 12;
	Buttons = buttons;
	State = new ExpPort_FamTrainerA_State;
	MovLen = 2;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->Sel = 0;
	State->NewBits = 0;
}
void ExpPort_FamTrainerA::CPUCycle (void) { }
} // namespace Controllers