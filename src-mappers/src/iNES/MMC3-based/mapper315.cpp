#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0x0F, reg <<3 &~0x0F);
	MMC3::syncCHR_ROM(0xFF, reg <<3 &0x040 | reg <<6 &0x080 | reg <<8 &0x100);
	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	if (reg &8)
		return MMC3::getPRGBank(bank &1) &~3 | bank &3;
	else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =315;
} // namespace

MapperInfo MapperInfo_315 = {
	&mapperNum,
	_T("830134C"),
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
