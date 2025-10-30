#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	if (reg &0x08)
		EMU->SetPRG_ROM32(0x8, reg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, reg);
		EMU->SetPRG_ROM16(0xC, reg);
	}
	MMC3::syncCHR_ROM(0xFF, 0x00);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg =val;
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

uint16_t mapperNum =259;
} // namespace

MapperInfo MapperInfo_259 = {
	&mapperNum,
	_T("F-15"),
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