#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace { // Like mapper 367 but with an added NROM mode
uint8_t reg;

void sync (void) {
	int prgAND =reg &0x02? 0x0F: 0x1F;
	int chrAND =reg &0x02? 0x7F: 0xFF;
	int prgOR =reg <<4;
	int chrOR =reg <<7;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	if ((reg &3) ==3 && ~reg &8)
		return MMC3::getPRGBank(bank &1) &~1 | bank &1;
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

uint16_t mapperNum =504;
} // namespace

MapperInfo MapperInfo_504 = {
	&mapperNum,
	_T("K-3054"),
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