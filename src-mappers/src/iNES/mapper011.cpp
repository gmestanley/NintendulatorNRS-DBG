#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data >>0);
	iNES_SetCHR_Auto8(0x0, Latch::data >>4);
	iNES_SetMirroring();
}

int     busConflict (int cpuData, int romData) { // 300 Ohm resistor placed so that for D0, ROM always wins.
	return romData &1 | cpuData &romData &~1;
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, ROM->INES_MapperNum ==144? busConflict: NULL); // mapper 11 has normal AND-type bus conflicts as well, but no game relies on them, and one prototype (Free Fall) does not work with them.
	return TRUE;
}

uint16_t mapperNum011 =11;
uint16_t mapperNum144 =144;
} // namespace

MapperInfo MapperInfo_011 ={
	&mapperNum011,
	_T("Color Dreams"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};

MapperInfo MapperInfo_144 = {
	&mapperNum144,
	_T("AGCI-50282"),
	COMPAT_FULL,
	load,
	Latch::reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
