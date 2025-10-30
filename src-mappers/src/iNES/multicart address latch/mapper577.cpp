#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &ROM->dipValue)
		for (int bank = 0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
	}
	EMU->SetCHR_ROM8(0x0, Latch::addr >>1);
	iNES_SetMirroring();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 577;
} // namespace

MapperInfo MapperInfo_577 = {
	&mapperNum,
	_T("KN-20"),
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
