#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data >>4);
	EMU->SetPRG_ROM16(0xC, 0xFF);
	EMU->SetCHR_ROM8(0, Latch::data &0xF);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, Latch::busConflictAND);
	return TRUE;
}

uint16_t mapperNum =70;
} // namespace

MapperInfo MapperInfo_070 ={
	&mapperNum,
	_T("Bandai UOROM"),
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