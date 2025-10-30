#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x10) {
		EMU->SetPRG_ROM16(0x8, Latch::addr <<1 | Latch::addr >>5 &1);
		EMU->SetPRG_ROM16(0xC, Latch::addr <<1 | Latch::addr >>5 &1);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr);
	if (ROM->INES2_SubMapper == 1)
		iNES_SetCHR_Auto8(0x0, Latch::addr >>7 &~0x01 | Latch::addr >>6 &0x01);
	else
		iNES_SetCHR_Auto8(0x0, Latch::addr >>8); 
	if (Latch::addr &0x80)
		EMU->Mirror_H();
	else 
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	EMU->GetCPUWriteHandler(0x0)(0x0, 0x01A, 0x00);
	EMU->GetCPUWriteHandler(0x0)(0x0, 0x01B, 0x00);
}

uint16_t mapperNum = 61;
} // namespace

MapperInfo MapperInfo_061 ={
	&mapperNum,
	_T("GS-2017/BS-N032"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};