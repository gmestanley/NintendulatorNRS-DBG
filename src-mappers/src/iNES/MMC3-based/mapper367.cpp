#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	int prgAND =reg &2? 0x0F: 0x1F;
	int chrAND =reg &2? 0x7F: 0xFF;
	int prgOR =reg <<4;
	int chrOR =reg <<7;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	if (ROM->dipValue && reg &1) reg |=2;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	reg =0;
	MMC3::reset(RESET_HARD);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =367;
} // namespace

MapperInfo MapperInfo_367 = {
	&mapperNum,
	_T("JC-016-2 variant"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};