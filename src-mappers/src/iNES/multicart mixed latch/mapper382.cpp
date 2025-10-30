#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	Latch::setLockedBits(Latch::addr &0x20? 0xFF: 0x00, 0x00);
	if (Latch::addr &0x08)
		EMU->SetPRG_ROM32(0x8, Latch::addr <<2| Latch::data &3);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr <<3| Latch::data &7);
		EMU->SetPRG_ROM16(0xC, Latch::addr <<3| 7);
	}
	EMU->SetCHR_RAM8(0x0, 0x0);
	if (Latch::addr &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, Latch::busConflictAND, true);
	return TRUE;
}

uint16_t mapperNum = 382;
} // namespace

MapperInfo MapperInfo_382 ={
	&mapperNum,
	_T("830928C"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};