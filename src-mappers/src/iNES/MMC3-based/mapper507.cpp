#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int prgAND = reg &0x20? 0x0F: 0x1F;
	int chrAND = reg &0x20? 0x7F: 0xFF;
	int prgOR  = reg;
	int chrOR  = reg <<3;
	MMC3::syncMirror();
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
}

int MAPINT readPad (int, int) {
	return ROM->dipValue;
}

void MAPINT writeReg (int, int, int val) {
	reg =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, readPad, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) { 
	reg =0;
	MMC3::reset(RESET_HARD);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 507;
} // namespace

MapperInfo MapperInfo_507 ={
	&mapperNum,
	_T("A018"),
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