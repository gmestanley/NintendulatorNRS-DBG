#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint16_t reg;

void sync (void) {
	if (reg &0x01)
		MMC3::syncPRG(0x1F, reg &0x60);
	else
		EMU->SetPRG_ROM32(0x8, reg >>2);
		
	MMC3::syncCHR(reg &0x100? 0x7F: 0xFF, reg >>1 &0x100 | reg &0x080);
	MMC3::syncMirror();
}

int MAPINT readPad (int bank, int addr) {
	return ROM->dipValue;
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg =addr ^0x1C;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, readPad, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0xFE;
	MMC3::reset(RESET_HARD);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_WORD(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =505;
} // namespace

MapperInfo MapperInfo_505 = {
	&mapperNum,
	_T("5426757A-Y2-230630"),
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
