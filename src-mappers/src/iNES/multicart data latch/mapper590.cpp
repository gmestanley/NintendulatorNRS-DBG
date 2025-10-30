#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::data &0x08)
		EMU->SetPRG_ROM32(0x8, Latch::data >>1);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::data);
		EMU->SetPRG_ROM16(0xC, Latch::data);
	}
	EMU->SetCHR_ROM8(0, Latch::data);
	iNES_SetMirroring();
}

BOOL MAPINT load (void) {
	Latch::load(sync, Latch::busConflictAND);
	return TRUE;
}

uint16_t mapperNum = 590;
} // namespace

MapperInfo MapperInfo_590 ={
	&mapperNum,
	_T("810430"),
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