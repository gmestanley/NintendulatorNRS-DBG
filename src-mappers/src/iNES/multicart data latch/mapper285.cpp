#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync0 (void) {
	if (Latch::data &0x40)
		EMU->SetPRG_ROM32(0x8, Latch::data >>1);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::data);
		EMU->SetPRG_ROM16(0xC, Latch::data |7);
	}
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x80) {
		if (Latch::data &0x20)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else {
		if (Latch::data &0x20)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
}

void sync1 (void) {
	if (Latch::data &0x40)
		EMU->SetPRG_ROM32(0x8, Latch::data >>1 &0x03 | Latch::data >>2 &~0x03);
	else {
		EMU->SetPRG_ROM16(0x8, Latch::data >>1 &~0x07 | Latch::data &0x07);
		EMU->SetPRG_ROM16(0xC, Latch::data >>1 &~0x07 |              0x07);
	}
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x80) {
		if (Latch::data &0x20)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else {
		if (Latch::data &0x08)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
}

int MAPINT readPad (int, int addr) {
	if (addr &0x80) return ROM->dipValue >=20? (4 | ROM->dipValue %20): 0; else
	if (addr &0x40) return ROM->dipValue >=16? (4 | ROM->dipValue %16): 0; else
	if (addr &0x20) return ROM->dipValue >=12? (4 | ROM->dipValue %12): 0; else
	if (addr &0x10) return ROM->dipValue >= 8? (4 | ROM->dipValue %8):  0; else
	return ROM->dipValue >=8? 0: ROM->dipValue &7;
}

BOOL MAPINT load (void) { 
	Latch::load(ROM->INES2_SubMapper == 1? sync1: sync0, NULL);
	MapperInfo_285.Description =ROM->INES2_SubMapper == 1? _T("JY-066"): _T("A65AS");
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	EMU->SetCPUReadHandler(0x5, readPad);
}

uint16_t mapperNum = 285;
} // namespace

MapperInfo MapperInfo_285 = {
	&mapperNum,
	_T("A65AS"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
