#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x08) {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1 &~1 | Latch::addr &1);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1 &~1 | Latch::addr &1);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>2);
	EMU->SetCHR_ROM8(0x0, Latch::addr >>1 &~1 | Latch::addr &1);
	if (Latch::addr &0x02)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 213;
} // namespace

MapperInfo MapperInfo_213 = {
	&mapperNum,
	_T("EJ-3003/820428-C"),
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
