#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x100) {
		if (Latch::addr &1) {
			EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
			EMU->SetPRG_ROM16(0xC, Latch::addr >>2);
		} else
			EMU->SetPRG_ROM32(0x8, Latch::addr >>3);
	} else {
		if (Latch::addr &1 && ROM->dipValue &1)
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
		else {
			EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
			EMU->SetPRG_ROM16(0xC, Latch::addr >>2 |7);
		}
	}
	EMU->SetCHR_ROM8(0, Latch::addr >>5 &7 | Latch::addr >>1 &8);
	if (Latch::addr &2)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum =494;
} // namespace

MapperInfo MapperInfo_494 = {
	&mapperNum,
	_T("CH512K/OK-103"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};