#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
int MAPINT readPad (int, int) {
	return ROM->dipValue &7 | *EMU->OpenBus &~7;
}

void sync (void) {
	if (Latch::data &0x80) {
		EMU->SetPRG_ROM16(0x8, Latch::data);
		EMU->SetPRG_ROM16(0xC, Latch::data); 
	} else
		EMU->SetPRG_ROM32(0x8, Latch::data >>1);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x40)
		EMU->Mirror_H();
	else 
		EMU->Mirror_V();
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, (Latch::addr &3) == 3? readPad: EMU->ReadPRG);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 482;
} // namespace

MapperInfo MapperInfo_482 = {
	&mapperNum,
	_T("K-1079"),
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