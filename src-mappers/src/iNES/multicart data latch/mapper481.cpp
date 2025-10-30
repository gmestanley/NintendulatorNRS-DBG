#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data >>4 &~7 | Latch::data &7);
	EMU->SetPRG_ROM16(0xC, Latch::data >>4 | 7);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 481;
} // namespace

MapperInfo MapperInfo_481 = {
	&mapperNum,
	_T("045N"),
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