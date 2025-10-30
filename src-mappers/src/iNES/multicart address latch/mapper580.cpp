#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x01) {
		EMU->SetPRG_ROM32(0x8, Latch::addr >>5);
		EMU->SetCHR_ROM8(0, Latch::addr >>3 &~3 | Latch::addr >>2 &3);
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>4);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>4);
		EMU->SetCHR_ROM8(0, Latch::addr >>3 &~1 | Latch::addr >>2 &1);
	}
	if (Latch::addr &0x02)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 580;
} // namespace

MapperInfo MapperInfo_580 = {
	&mapperNum,
	_T("ET-156"),
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