#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	Latch::setLockedBits(0x0000, Latch::data &0x20? (Latch::data &0x08? 0x28: 0x2C): 0x00);
	EMU->SetCHR_RAM8(0x0, 0); 
	EMU->SetPRG_ROM32(0x8, Latch::data);
	if (Latch::data &0x20) {
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else
		if (Latch::data &0x10)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

uint16_t mapperNum = 564;
} // namespace

MapperInfo MapperInfo_564 = {
	&mapperNum,
	_T("bd23.pcb"),
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
