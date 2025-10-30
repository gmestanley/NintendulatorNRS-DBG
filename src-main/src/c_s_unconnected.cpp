#include "stdafx.h"
#include "Nintendulator.h"
#include "Settings.h"
#include "Controllers.h"

namespace Controllers
{
int	StdPort_Unconnected::Save (FILE *out)
{
	int clen = 0;

	writeWord(0);

	return clen;
}
int	StdPort_Unconnected::Load (FILE *in, int version_id)
{
	int clen = 0;
	unsigned short len;

	readWord(len);
	fseek(in, len, SEEK_CUR);

	return clen;
}
void	StdPort_Unconnected::Frame (unsigned char mode)
{
}
unsigned char	StdPort_Unconnected::Read (void)
{
	return 0;
}
void	StdPort_Unconnected::Write (unsigned char Val)
{
}
void	StdPort_Unconnected::Config (HWND hWnd)
{
	MessageBox(hWnd, _T("No configuration necessary!"), _T("Nintendulator"), MB_OK);
}
void	StdPort_Unconnected::SetMasks (void)
{
}
StdPort_Unconnected::~StdPort_Unconnected (void)
{
}
StdPort_Unconnected::StdPort_Unconnected (DWORD *buttons)
{
	Type = STD_UNCONNECTED;
	NumButtons = 0;
	Buttons = buttons;
	State = NULL;
	MovLen = 0;
	MovData = NULL;
}

void StdPort_Unconnected::CPUCycle (void) { }
} // namespace Controllers