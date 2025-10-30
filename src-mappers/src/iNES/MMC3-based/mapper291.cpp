#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	if (reg &0x20)
		EMU->SetPRG_ROM32(0x8, reg >>1 &0x1E | reg >>4 &0x04);
	else
		MMC3::syncPRG(0x0F, reg >>2 &0x10);
	MMC3::syncCHR_ROM(0xFF, reg <<2 &0x100);
	MMC3::syncMirror();
}

void MAPINT writeReg (int, int, int val) {
	reg =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType == RESET_HARD) reg =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =291;
} // namespace

MapperInfo MapperInfo_291 = {
	&mapperNum,
	_T("Super 2-in-1"),
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