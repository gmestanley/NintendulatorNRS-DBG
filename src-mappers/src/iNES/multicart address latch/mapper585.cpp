#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	int prg = Latch::addr >>1;
	if (Latch::addr &ROM->dipValue)
		for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetPRG_OB4(bank);
	else
	if (Latch::addr &1)
		EMU->SetPRG_ROM32(0x8, prg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	}
	EMU->SetCHR_ROM8(0, prg);
	if (Latch::addr &0x10)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 585;
} // namespace

MapperInfo MapperInfo_585 = {
	&mapperNum,
	_T("FE-01-1"),
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