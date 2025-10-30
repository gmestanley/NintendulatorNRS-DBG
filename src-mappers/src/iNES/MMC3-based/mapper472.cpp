#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	MMC3::syncPRG(0x0F, reg &0xF0);
	if (reg &0x20)
		MMC3::syncCHR_ROM(0x7F, reg <<3 &0x780);
	else
		MMC3::syncCHR_ROM(0xFF, reg <<3 &0x700);
	MMC3::syncWRAM(0);
	MMC3::syncMirror();
}

int	MAPINT	readPad (int bank, int addr) {
	return ROM->dipValue;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, readPad, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	reg =0;
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =472;
} // namespace

MapperInfo MapperInfo_472 ={
	&mapperNum,
	_T("830947"),
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