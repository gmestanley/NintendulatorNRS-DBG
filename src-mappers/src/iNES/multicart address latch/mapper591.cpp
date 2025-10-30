#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if ((Latch::addr &0xC) == 0xC)
		EMU->SetPRG_ROM32(0x8, Latch::addr >> 2);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1);
	}
	EMU->SetCHR_ROM8(0, Latch::addr);
	if (Latch::addr &0x08)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t MapperNum = 591;
} // namespace

MapperInfo MapperInfo_591 = {
	&MapperNum,
	_T("07027/810543"),
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