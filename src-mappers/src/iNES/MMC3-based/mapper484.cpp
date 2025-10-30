#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0x1F, reg <<5);
	MMC3::syncCHR(0xFF, 0x00);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (~reg &0x80) reg = val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg = 0;
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 484;
} // namespace

MapperInfo MapperInfo_484 ={
	&mapperNum,
	_T("MMC3 2x256 KiB"),
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
