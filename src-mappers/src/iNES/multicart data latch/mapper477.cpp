#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::data &0xE) {
		EMU->SetPRG_ROM16(0x8, 0xFF);
		EMU->SetPRG_ROM16(0xC, Latch::data);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::data >>1);
	EMU->SetCHR_ROM8(0x0, Latch::data);
	iNES_SetMirroring();
}

int busConflict (int cpuData, int romData) { // Resistors placed on the first 128 KiB PRG ROM chip (banks 8-15) cause ROM to always win D0..D3. Otherwise, normal AND-type bus conflicts.
	return Latch::data &8? (romData &15 | cpuData &romData &~15): (cpuData &romData);
}

BOOL MAPINT load (void) {
	Latch::load(sync, busConflict);
	return TRUE;
}

uint16_t mapperNum = 477;
} // namespace

MapperInfo MapperInfo_477 = {
	&mapperNum,
	_T("15-in-1"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};