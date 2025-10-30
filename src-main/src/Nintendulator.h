/* NintendulatorNRS+DBG
 * Copyright (C) 2025 gmestanley
 *
 * based on NintendulatorDX
 * Copyright (C) 2015 fo-fo
 *
 * and NintendulatorNRS
 * Copyright (C) 2017-2023 NewRisingSun
 *
 * based on Nintendulator - Win32 NES emulator written in C++
 * Copyright (C) 2002-2019 QMT Productions
 *
 * Based on NinthStar, a portable Win32 NES Emulator written in C++
 * Copyright (C) 2000  David de Regt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include <set>

#define SAMPLING_RATE 48000 // 48000, 192000
#define DISABLE_ALL_FILTERS 1
#define SUPPORT_STEREO 0
#define	ENABLE_DEBUGGER	     // Enable the debugger - emulation is faster without it
//#define	CPU_BENCHMARK	// Run cyctest.nes for 4542110 cycles (10 seconds), then report how long it took
//#define	SHORQ	// Enable ShoRQ(tm) technology - enable green color emphasis whenever there's an active IRQ

extern	int		ConfigVersion;	// Configuration version - used if files get moved around or settings change meaning

extern	HINSTANCE	hInst;	/* current instance */
extern	HWND		hMainWnd;	/* main window */
extern	HMENU		hMenu;	/* main window menu */
extern	HACCEL		hAccelTable;	/* accelerators */
extern std::set<HWND>   hWindows; // Handles for modeless windows from the mapper DLL

extern	BOOL		MaskKeyboard;	/* mask keyboard accelerators (for when Family Basic Keyboard is active) */
extern	BOOL		MaskMouse;	/* hide mouse cursor (for Arkanoid paddle and SNES Mouse) */
extern	HWND		hDebug;		/* Debug Info window */
extern	HCURSOR		CursorNormal, CursorCross, *CurrentCursor;
extern	BOOL		CursorOnOutput;
extern	BOOL		prev_multiCanSave;

extern	TCHAR		ProgPath[MAX_PATH];	/* program path */
extern	TCHAR		DataPath[MAX_PATH];	/* data path */

extern	TCHAR		Path_ROM[MAX_PATH];
extern	TCHAR		Path_NMV[MAX_PATH];
extern	TCHAR		Path_AVI[MAX_PATH];
extern	TCHAR		Path_PAL[MAX_PATH];

extern	void		BrowseFolder (const TCHAR *dir);

extern	BOOL		ProcessMessages	(void);

extern	TCHAR		TitlebarBuffer[256];
extern	int		TitlebarDelay;
extern	void		UpdateTitlebar (void);
extern	void	__cdecl	PrintTitlebar (const TCHAR *Text, ...);
extern	void		AddDebug (const TCHAR *txt);
extern  void	UpdateInterface (void);

// Shortcut macros for use in savestate code
#define	writeByte(val) { register unsigned char _val = val; fwrite(&_val, 1, 1, out); clen++; }
#define	writeWord(val) { register unsigned short _val = val; fwrite(&_val, 2, 1, out); clen += 2; }
#define	writeLong(val) { register unsigned long _val = val; fwrite(&_val, 4, 1, out); clen += 4; }
#define	write64(val)   { register unsigned long long _val = val; fwrite(&_val, 8, 1, out); clen += 8; }
#define	writeBool(val) { register bool _val = val; fwrite(&_val, sizeof(bool), 1, out); clen += sizeof(bool); }
#define	writeArray(val,len) { register int _len = len; fwrite(val, 1, _len, out); clen += _len; }

#define	readByte(val) { register unsigned char _val; fread(&_val, 1, 1, in); val = _val; clen++; }
#define	readWord(val) { register unsigned short _val; fread(&_val, 2, 1, in); val = _val; clen += 2; }
#define	readLong(val) { register unsigned long _val; fread(&_val, 4, 1, in); val = _val; clen += 4; }
#define	read64(val)   { register unsigned long long _val; fread(&_val, 8, 1, in); val = _val; clen += 8; }
#define	readBool(val) { register bool _val; fread(&_val, sizeof(bool), 1, in); val = _val; clen += sizeof(bool); }
#define	readArray(val,len) { register int _len = len; fread(val, 1, _len, in); clen += _len; }
#define	readArraySkip(val,inlen,outlen) { register int readLen = min(inlen, outlen); fread(val, 1, readLen, in); if (inlen > readLen) fseek(in, inlen - readLen, SEEK_CUR); if (outlen > readLen) ZeroMemory((unsigned char *)val + readLen, outlen - readLen); clen += inlen; }
