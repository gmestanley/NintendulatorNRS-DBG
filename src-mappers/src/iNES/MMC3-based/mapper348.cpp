#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	//int outerBank =(reg >>2) &3;
	//if ((reg &0xC) ==0xC)
	MMC3::syncPRG(0x0F, reg <<2 &~0x0F);
	MMC3::syncCHR_ROM(0x7F, reg <<5 &~0x7F);
	MMC3::syncMirror();
}

int getPRGPage (int bank) {
	if ((reg &0xC) ==0xC)
		return MMC3::getPRGBank(bank &1) &~3 | bank &3;
	else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int, int, int val) {
	reg =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGPage, NULL, NULL, writeReg);
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

uint16_t mapperNum =348;
} // namespace

MapperInfo MapperInfo_348 = {
	&mapperNum,
	_T("830118C"),
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