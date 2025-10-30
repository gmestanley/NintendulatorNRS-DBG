#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0x0F, reg <<4);	

	int chrAND = reg &0x02? 0xFF: 0x7F;
	int chrOR  = reg << 7;
	if (reg &0x04) {
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) &chrAND | chrOR &~chrAND);
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(3) &chrAND | chrOR &~chrAND);
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) &chrAND | chrOR &~chrAND);
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7) &chrAND | chrOR &~chrAND);
	} else
		MMC3::syncCHR(chrAND, chrOR &~chrAND);
	
	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	if (reg &0x08)
		return MMC3::getPRGBank(0) &~3 | bank &3;
	else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int, int addr, int) {
	reg = addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::Sharp, getPRGBank, NULL, NULL, writeReg); // Super Fighter III has stray 6xxx writes that must be ignored
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

uint16_t mapperNum = 569;
} // namespace

MapperInfo MapperInfo_569 ={
	&mapperNum,
	_T("820315-C"),
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

