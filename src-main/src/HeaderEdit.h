#pragma once

namespace HeaderEdit {
#define NUM_EXP_DEVICES 0x4F
extern	void	Open (const TCHAR *);
extern	const	TCHAR *ConsoleTypes[16];
extern	const	TCHAR *ExpansionDevices[NUM_EXP_DEVICES];
extern	const	TCHAR *VSFlags[16];
} // namespace HeaderEdit
