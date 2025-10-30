#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::data &0x20) {
		EMU->SetPRG_ROM16(0x8, 0x10 | Latch::data <<1 &0x0E | Latch::data >>3 &0x01);
		EMU->SetPRG_ROM16(0xC, 0x10 | Latch::data <<1 &0x08 |                  0x07);
		if (Latch::data &0x04)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	} else {
		EMU->SetPRG_ROM32(0x8, Latch::data &0x07);
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	}
	EMU->SetCHR_RAM8(0x0, 0);
}

BOOL MAPINT load (void) {
	Latch::load(sync, Latch::busConflictAND);
	return TRUE;
}

uint16_t mapperNum = 378;
} // namespace

MapperInfo MapperInfo_378 ={
	&mapperNum,
	_T("8-in-1 AOROM+UNROM"),
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
