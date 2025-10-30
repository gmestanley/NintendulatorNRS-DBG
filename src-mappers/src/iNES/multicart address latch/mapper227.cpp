#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
int MAPINT readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr | ROM->dipValue);
}

void sync (void) {
	int prg = Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20;
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0x0, 0);
	if (Latch::addr &0x080) {
		if (Latch::addr &0x001)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
		if (ROM->INES2_SubMapper != 0) protectCHRRAM();
	} else {
		EMU->SetPRG_ROM16(0x8, prg);
		if (ROM->INES2_SubMapper ==3) // JC-007
			EMU->SetPRG_ROM16(0xC, 0);
		else
		if (ROM->INES2_SubMapper ==2)
			EMU->SetPRG_ROM16(0xC, Latch::addr &0x200? (prg |7): 0);
		else
			EMU->SetPRG_ROM16(0xC, Latch::addr &0x200? (prg |7): (prg &~7));
	}
	if (Latch::addr &0x002)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();

	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, Latch::addr &0x400? readPad: EMU->ReadPRG);
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum = 227;
} // namespace

MapperInfo MapperInfo_227 = {
	&mapperNum,
	_T("外星 FW01"),
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
