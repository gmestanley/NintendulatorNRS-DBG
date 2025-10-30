#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x40) {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
	EMU->SetCHR_ROM8(0x0, Latch::addr >>3);
	if (Latch::addr &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 58;
} // namespace

MapperInfo MapperInfo_058 = {
	&mapperNum,
	_T("GK-192"),
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
