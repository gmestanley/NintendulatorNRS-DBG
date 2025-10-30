#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "GFX.h"
#include "MapperInterface.h"

namespace Controllers {
#include <pshpack1.h>
struct ExpPort_ArkanoidPaddle_State {
	unsigned char Bits;
	unsigned short Pos;
	unsigned char BitPtr;
	unsigned char Strobe;
	unsigned char Button;
	unsigned char NewBits;
	unsigned char Bits2;
	unsigned short Pos2;
	unsigned char BitPtr2;
	unsigned char Button2;
	unsigned char NewBits2;
};
#include <poppack.h>
int	ExpPort_ArkanoidPaddle::Save (FILE *out) {
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_ArkanoidPaddle::Load (FILE *in, int version_id) {
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_ArkanoidPaddle::Frame (unsigned char mode) {
	int x, i, bits;
	if (mode & MOV_PLAY) {
		State->Pos = MovData[0] | ((MovData[1] << 8) & 0x7F);
		State->Button = MovData[1] >> 7;
		State->Pos2 = MovData[2] | ((MovData[3] << 8) & 0x7F);
		State->Button2 = MovData[3] >> 7;
	} else {
		State->Button = IsPressed(Buttons[0]);
		State->Pos += GetDelta(Buttons[2]);
		if (RI.InputType ==INPUT_VAUS_PROTOTYPE) {
			if (State->Pos < 38) State->Pos =38;
			if (State->Pos >228) State->Pos =228;
		} else {
			// Arkanoid's expected range is 196-484 (code caps to 196-516)
			// Arkanoid 2 SP expected range is 156-452 (code caps to 156-372/420/452/484)
			// Arkanoid 2 VS expected range is 168-438
			if (State->Pos <156) State->Pos =156;
			if (State->Pos >484) State->Pos =484;
		}
		State->Button2 = IsPressed(Buttons[1]);
		State->Pos2 += GetDelta(Buttons[3]);
		if (RI.InputType ==INPUT_VAUS_PROTOTYPE) {
			if (State->Pos2 < 38) State->Pos2 =38;
			if (State->Pos2 >228) State->Pos2 =228;
		} else {
			// Arkanoid's expected range is 196-484 (code caps to 196-516)
			// Arkanoid 2 SP expected range is 156-452 (code caps to 156-372/420/452/484)
			// Arkanoid 2 VS expected range is 168-438
			if (State->Pos2 <156) State->Pos2 =156;
			if (State->Pos2 >484) State->Pos2 =484;
		}
	}
	if (mode & MOV_RECORD) {
		MovData[0] = (unsigned char)(State->Pos & 0xFF);
		MovData[1] = (unsigned char)((State->Pos >> 8) | (State->Button << 7));
		MovData[2] = (unsigned char)(State->Pos2 & 0xFF);
		MovData[3] = (unsigned char)((State->Pos2 >> 8) | (State->Button2 << 7));
	}
	bits = 0;
	x = ~State->Pos;
	for (i =0; i <(RI.InputType ==INPUT_VAUS_PROTOTYPE? 8: 9); i++) {
		bits <<=1;
		bits |= x &1;
		x >>=1;
	}
	State->NewBits = bits;
	
	bits = 0;
	x = ~State->Pos2;
	for (i =0; i <(RI.InputType ==INPUT_VAUS_PROTOTYPE? 8: 9); i++) {
		bits <<=1;
		bits |= x &1;
		x >>=1;
	}
	State->NewBits2 = bits;
}
unsigned char	ExpPort_ArkanoidPaddle::Read1 (void) {
	if (RI.InputType ==INPUT_VAUS_PROTOTYPE) {
		if (State->BitPtr <7)
			return (unsigned char) State->Bits >>State->BitPtr++ <<1 &2;
		else	
			return 0x02;		
	} else
		return State->Button? 0x02: 0x00;
}
unsigned char	ExpPort_ArkanoidPaddle::Read2 (void) {
	int result =0;
	
	if (RI.InputType ==INPUT_VAUS_PROTOTYPE) {
		if (State->BitPtr2 <7)
			result |=State->Bits2 >>State->BitPtr2++ <<1 &2;
		else
			result |=2;
		if (State->Button) result |=8;
		if (State->Button2) result |=16;
	} else {
		if (State->BitPtr <8)
			result |=State->Bits >> State->BitPtr++ <<1 &2;
		else
			result |=2;
		if (State->BitPtr2 <8)
			result |=State->Bits2 >> State->BitPtr2++ <<4 &16;
		else
			result |=16;
		if (State->Button2) result |=8;
	}
	return result;
}
unsigned char	ExpPort_ArkanoidPaddle::ReadIOP (uint8_t) {
	return 0;
}
void	ExpPort_ArkanoidPaddle::Write (unsigned char Val) {
	if ((!State->Strobe) && (Val & 1)) 	{
		State->Bits = State->NewBits;
		State->BitPtr = 0;
		State->Bits2 = State->NewBits2;
		State->BitPtr2 = 0;
	}
	State->Strobe = Val & 1;
}
INT_PTR	CALLBACK	ExpPort_ArkanoidPaddle_ConfigProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int dlgLists[4] = {IDC_CONT_D0,IDC_CONT_D1,IDC_CONT_D2,IDC_CONT_D3};
	const int dlgButtons[4] = {IDC_CONT_K0,IDC_CONT_K1,IDC_CONT_K2,IDC_CONT_K3};
	ExpPort *Cont;
	if (uMsg == WM_INITDIALOG) {
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Cont = (ExpPort *)lParam;
	} else
		Cont = (ExpPort *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	return ParseConfigMessages(hDlg, uMsg, wParam, lParam, 2, 2, dlgLists, dlgButtons, Cont? Cont->Buttons: NULL, false);
}
void	ExpPort_ArkanoidPaddle::Config (HWND hWnd) {
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EXPPORT_ARKANOIDPADDLE), hWnd, ExpPort_ArkanoidPaddle_ConfigProc, (LPARAM)this);
}
void	ExpPort_ArkanoidPaddle::SetMasks (void) {
	MaskMouse = ((Buttons[1] >> 16) == 1);
}
ExpPort_ArkanoidPaddle::~ExpPort_ArkanoidPaddle (void) {
	delete State;
	delete[] MovData;
}
ExpPort_ArkanoidPaddle::ExpPort_ArkanoidPaddle (DWORD *buttons) {
	Type = EXP_ARKANOIDPADDLE;
	NumButtons = 4;
	Buttons = buttons;
	State = new ExpPort_ArkanoidPaddle_State;
	MovLen = 4;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Bits = 0;
	State->Pos = 340;
	State->BitPtr = 0;
	State->Strobe = 0;
	State->Button = 0;
	State->NewBits = 0;
	State->Bits2 = 0;
	State->Pos2 = 340;
	State->BitPtr2 = 0;
	State->Button2 = 0;
	State->NewBits2 = 0;
}
void ExpPort_ArkanoidPaddle::CPUCycle (void) { }
} // namespace Controllers