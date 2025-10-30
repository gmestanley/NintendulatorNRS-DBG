#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	int prg = Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20 | Latch::addr >>4 &0x40;
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0x0, 0); 
	if (Latch::addr &0x200) {
		if (Latch::addr &0x001) {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		} else
			EMU->SetPRG_ROM32(0x8, prg >>1);
		protectCHRRAM();
	} else
	if (Latch::addr &(ROM->INES2_SubMapper == 1? 0x001: 0x0400) && ROM->dipValue)
		for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetPRG_OB4(bank);
	else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg | 7);
	}
	if (Latch::addr &0x002)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 435;
} // namespace


MapperInfo MapperInfo_435 = {
	&mapperNum,
	_T("F-1002"),
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