#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0x0F, reg <<4);
	if (reg == 0x0F) {
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) | 0x200);
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(1) | 0x200);
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) | 0x200);
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7) | 0x200);
	} else
		MMC3::syncCHR(0x7F, reg <<7 &0x380);
	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	if (reg == 0x08)
		return MMC3::getPRGBank(bank &1) &~3 | bank &3;
	else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int, int addr, int) {
	reg =addr;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::Sharp, getPRGBank, NULL, NULL, writeReg); // Sharp because of stray 6xxx writes
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg = 0;
	MMC3::reset(resetType); 
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode == STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum = 567;
} // namespace

MapperInfo MapperInfo_567 ={
	&mapperNum,
	_T("Top Ten Variety (Super Fighter III)"),
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