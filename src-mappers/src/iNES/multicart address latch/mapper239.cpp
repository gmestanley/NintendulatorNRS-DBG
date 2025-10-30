#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x04)
		EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
	}
	EMU->SetCHR_ROM8(0x0, Latch::addr);
	if (Latch::addr &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 239;
} // namespace

MapperInfo MapperInfo_239 = {
	&mapperNum,
	_T("OK-043"),
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
