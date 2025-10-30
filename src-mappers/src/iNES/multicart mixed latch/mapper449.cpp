#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
int MAPINT readPad5 (int, int) {
	return ROM->dipValue;
}

int MAPINT readPad8 (int bank, int addr) {
	return EMU->ReadPRG(bank, addr | ROM->dipValue);
}

void sync (void) {
	int prg = Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20 | Latch::addr >>4 &0x40;
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0x0, Latch::data);
	if (Latch::addr &0x080) {
		if (Latch::addr &0x001)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
		if (ROM->INES2_SubMapper == 0) protectCHRRAM();
	} else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg |7);
	}
	if (Latch::addr &0x002)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();

	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, ROM->INES2_SubMapper == 0 && Latch::addr &0x200? readPad8: EMU->ReadPRG);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	if (ROM->INES2_SubMapper == 1) EMU->SetCPUReadHandler(0x5, readPad5);
}

uint16_t mapperNum = 449;
} // namespace


MapperInfo MapperInfo_449 = {
	&mapperNum,
	_T("22-in-1 King Series"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};