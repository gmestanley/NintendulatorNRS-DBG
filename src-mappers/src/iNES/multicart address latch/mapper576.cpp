#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x100) {
		EMU->SetPRG_ROM16(0x8, Latch::addr >>4);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>4);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>5);
	iNES_SetCHR_Auto8(0x0, Latch::addr); 
	if (Latch::addr &0x200)
		EMU->Mirror_H();
	else 
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 576;
} // namespace

MapperInfo MapperInfo_576 ={
	&mapperNum,
	_T("J-2096"),
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