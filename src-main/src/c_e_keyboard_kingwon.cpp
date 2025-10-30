#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "resource.h"
#include "Movie.h"
#include "Controllers.h"
#include "MapperInterface.h"

namespace Controllers
{
#include <pshpack1.h>
struct ExpPort_KingwonKeyboard_State {
	unsigned char Column;
	unsigned char Row;
	unsigned char Keys[13];
	unsigned char Bits;
};
#include <poppack.h>
int	ExpPort_KingwonKeyboard::Save (FILE *out)
{
	int clen = 0;
	unsigned short len = sizeof(*State);

	writeWord(len);
	writeArray(State, len);

	return clen;
}
int	ExpPort_KingwonKeyboard::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	readArraySkip(State, len, sizeof(*State));

	return clen;
}
void	ExpPort_KingwonKeyboard::Frame (unsigned char mode)
{
	int row, col;
	if (mode & MOV_PLAY)
	{
		for (row = 0; row < 13; row++)
			State->Keys[row] = MovData[row];
	}
	else
	{
		const int keymap[13][8] = {
			{DIK_Z,DIK_A,DIK_Q,DIK_1,DIK_LSHIFT,DIK_CAPITAL,DIK_TAB,DIK_GRAVE},
			{DIK_C,DIK_D,DIK_E,DIK_3,DIK_X,DIK_S,DIK_W,DIK_2},
			{DIK_B,DIK_G,DIK_T,DIK_5,DIK_V,DIK_F,DIK_R,DIK_4},
			{DIK_M,DIK_J,DIK_U,DIK_7,DIK_N,DIK_H,DIK_Y,DIK_6},
			{DIK_PERIOD,DIK_L,DIK_O,DIK_9,DIK_COMMA,DIK_K,DIK_I,DIK_8},
			{DIK_BACKSLASH,DIK_APOSTROPHE,DIK_LBRACKET,DIK_MINUS,DIK_SLASH,DIK_SEMICOLON,DIK_P,DIK_0,},
			{DIK_END,DIK_LEFT,DIK_HOME,0,DIK_BACK,DIK_RETURN,DIK_RBRACKET,DIK_EQUALS},
			{DIK_NEXT,DIK_RIGHT,DIK_PRIOR,DIK_ESCAPE,DIK_DOWN,0,DIK_UP,0,},
			{DIK_DELETE,DIK_ADD,DIK_MULTIPLY,DIK_NUMPADENTER,DIK_INSERT,DIK_SPACE,DIK_LMENU,DIK_LCONTROL},
			{DIK_F8,DIK_F7,DIK_F6,DIK_F5,DIK_F4,DIK_F3,DIK_F2,DIK_F1},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0}
		};

		for (row = 0; row < 13; row++)
		{
			State->Keys[row] = 0;
			for (col = 0; col < 8; col++)
				if (KeyState[keymap[row][col]] & 0x80)
					State->Keys[row] |= 1 << col;
		}
		/*// special cases
		if (KeyState[DIK_RSHIFT] & 0x80)
			State->Keys[7] |= 0x80;
		if (KeyState[DIK_RCONTROL] & 0x80)
			State->Keys[5] |= 0x80;*/
	}
	if (mode & MOV_RECORD)
	{
		for (row = 0; row < 13; row++)
			MovData[row] = State->Keys[row];
	}
}
unsigned char	ExpPort_KingwonKeyboard::Read1 (void) {
	return 0;
}
unsigned char	ExpPort_KingwonKeyboard::Read2 (void)
{
	unsigned char result = 0xFF;
	if (State->Row < 13) 	{
		if (State->Keys[State->Row] & (1 <<State->Column)) result &=~2;
		State->Column =(State->Column +1) &7;
	}
	return result;
}

unsigned char	ExpPort_KingwonKeyboard::ReadIOP (uint8_t) {
	return 0;
}

void	ExpPort_KingwonKeyboard::Write (unsigned char val) {
	// 0,2,|3,2,6,2
	if (val &4) { // Select keyboard
		if (State->Bits &2 && ~val &2) { // Reset keyboard
			State->Row =0;
			State->Column =0;
		} else
		if (State->Bits &1 && ~val &1) { // Next row
			State->Row =++State->Row %13;
			State->Column =0;
		}
		State->Bits =val;
	}
}
void	ExpPort_KingwonKeyboard::Config (HWND hWnd)
{
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}
void	ExpPort_KingwonKeyboard::SetMasks (void)
{
	MaskKeyboard = TRUE;
}
ExpPort_KingwonKeyboard::~ExpPort_KingwonKeyboard (void)
{
	delete State;
	delete[] MovData;
}
ExpPort_KingwonKeyboard::ExpPort_KingwonKeyboard (DWORD *buttons)
{
	Type = EXP_KINGWONKEYBOARD;
	NumButtons = 0;
	Buttons = buttons;
	State = new ExpPort_KingwonKeyboard_State;
	MovLen = 13;
	MovData = new unsigned char[MovLen];
	ZeroMemory(MovData, MovLen);
	State->Row = 0;
	State->Column = 0;
	State->Bits = 0;
	ZeroMemory(State->Keys, sizeof(State->Keys));
}
void ExpPort_KingwonKeyboard::CPUCycle (void) { 
}
} // namespace Controllers