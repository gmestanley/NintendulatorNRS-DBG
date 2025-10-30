#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	if (reg &0x10)
		MMC3::syncPRG(0x0F, reg <<2 &~0x0F);
	else
		EMU->SetPRG_ROM32(0x8, reg);
	MMC3::syncCHR_ROM(0x7F, reg <<5 &~0x7F);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
}

void MAPINT writeReg (int, int, int val) {
	reg =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
	for (int bank =0x9; bank<=0xF; bank+=0x2) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =333;
} // namespace

MapperInfo MapperInfo_333 = {
	&mapperNum,
	_T("GRM070"),
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