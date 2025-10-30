#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
	EMU->SetPRG_ROM16(0xC, Latch::addr >>2);
	EMU->SetCHR_ROM8(0, Latch::addr >>2);
	if (Latch::addr &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 300;
} // namespace

MapperInfo MapperInfo_300 ={
	&mapperNum,
	_T("190-in-1"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};