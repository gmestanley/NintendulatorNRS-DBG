#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[2];

void sync (void) {
	int prgAND = reg[0] &0x80? 0x0F: 0x1F;
	int chrAND = 0xFF;
	int prgOR  = reg[0] >>1 &0x20 | reg[1] <<4 &0x10;
	int chrOR  = reg[0] <<2 &0x100;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[addr &1] = val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& c: reg) c = 0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 537;
} // namespace

MapperInfo MapperInfo_537 ={
	&mapperNum,
	_T("JY4M4"),
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
