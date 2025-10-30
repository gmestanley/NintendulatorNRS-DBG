﻿#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
void sync (void) {
	MMC3::syncWRAM(0);
	MMC3::syncPRG(0x3F, 0);
	for (int bank =0; bank <8; bank++) {
		int val =MMC3::getCHRBank(bank);
		if ((val &~3) ==0x8)
			EMU->SetCHR_RAM1(bank, val &3);
		else
			EMU->SetCHR_ROM1(bank, val);
	}
	MMC3::syncMirror();
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

uint16_t mapperNum =192;
} // namespace

MapperInfo MapperInfo_192 ={
	&mapperNum,
	_T("外星 FS308"),
	COMPAT_FULL,
	load,
	MMC3::reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};
