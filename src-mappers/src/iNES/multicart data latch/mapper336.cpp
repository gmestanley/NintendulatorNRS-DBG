#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data);
	EMU->SetPRG_ROM16(0xC, Latch::data |7);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &(ROM->INES2_SubMapper ==2? 0x08: 0x20))
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

int     busConflict (int cpuData, int romData) { // 300 Ohm resistor placed so that for D3, ROM always wins.
	return romData &8 | cpuData &romData &~8;
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, ROM->INES2_SubMapper ==1? busConflict: NULL);
	return TRUE;
}

uint16_t mapperNum =336;
} // namespace

MapperInfo MapperInfo_336 = {
	&mapperNum,
	_T("K-3046"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};