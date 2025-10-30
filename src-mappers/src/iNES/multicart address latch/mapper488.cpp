#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
int MAPINT readSolderPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~0xF | ROM->dipValue &0xF);
}

void sync (void) {
	if (Latch::addr &0x4)
		EMU->SetPRG_ROM32(0x8, Latch::addr);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::addr <<1 | Latch::addr >>4 &0x1);
		EMU->SetPRG_ROM16(0xC, Latch::addr <<1 | Latch::addr >>4 &0x1);
	}
	iNES_SetCHR_Auto8(0x0, Latch::addr);
	iNES_SetMirroring();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, Latch::addr &0x100? readSolderPad: EMU->ReadPRG);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum =488;
} // namespace

MapperInfo MapperInfo_488 ={
	&mapperNum,
	_T("HC001"),
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