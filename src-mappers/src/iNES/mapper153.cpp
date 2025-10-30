#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FCG.h"

namespace {
uint8_t		outerBank;

void	sync (void) {
	FCG::syncPRG(0x0F, outerBank <<4);
	EMU->SetCHR_RAM8(0x0, 0);
	FCG::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	FCG::load(sync, FCGType::LZ93D50, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) outerBank =0;
	FCG::reset(resetType);
}

void	MAPINT	ppuCycle (int addr, int, int, int) {
	uint8_t newOuterBank =FCG::chr[addr >>10 &3];
	if (newOuterBank !=outerBank) {
		outerBank =newOuterBank;
		sync();
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =FCG::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	return offset;
}

uint16_t mapperNum =153;
} // namespace

MapperInfo MapperInfo_153 ={
	&mapperNum,
	_T("Bandai FCG with 8 KiB PRG-RAM"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	FCG::cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};