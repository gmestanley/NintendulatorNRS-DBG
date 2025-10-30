#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_N118.h"

namespace {
void	sync (void) {
	N118::syncPRG(0x0F, 0x00);
	N118::syncCHR(0x3F, 0x00);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int, int addr, int val) {
	N118::reg[addr &7] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	N118::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	N118::reset(resetType);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}


uint16_t mapperNum =486;
} // namespace

MapperInfo MapperInfo_486 ={
	&mapperNum,
	_T("Kaiser KS-7009"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	N118::saveLoad,
	NULL,
	NULL
};