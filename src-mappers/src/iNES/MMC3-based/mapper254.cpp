#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
bool protect;

void sync (void) {
	MMC3::syncWRAM(0);
	MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR(0xFF, 0x00);
	MMC3::syncMirror();
}

int MAPINT readWRAM (int bank, int addr) {
	return EMU->ReadPRG(bank, addr) ^(protect *0x80);
}

void MAPINT writeWRAM (int bank, int addr, int val) {
	if (val &1) protect =false;
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, readWRAM, writeWRAM);
	iNES_SetSRAM();
	return true;
}

void MAPINT reset (RESET_TYPE resetType) {
	protect =true;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BOOL(stateMode, offset, data, protect);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 254;
} // namespace

MapperInfo MapperInfo_254 = {
	&mapperNum,
	_T("Pikachu Y2K"),
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
