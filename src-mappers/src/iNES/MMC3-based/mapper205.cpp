#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int prgAND =reg &0x02? 0x0F: 0x1F;
	int chrAND =reg &0x02? 0x7F: 0xFF;
	MMC3::syncPRG(prgAND, reg <<4 &~prgAND);
	MMC3::syncCHR_ROM(chrAND, reg <<7 &~chrAND);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg =val;
	if (reg &0x01 && ROM->dipValue) reg |=0x02;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =205;
} // namespace

MapperInfo MapperInfo_205 = {
	&mapperNum,
	_T("JC-016-2"),
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