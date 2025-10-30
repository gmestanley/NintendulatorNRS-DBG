#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

int MAPINT readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~0x0F | ROM->dipValue &0x0F);
}

void sync (void) {
	int prgAND = reg &0x20? 0x1F: 0x0F;
	int chrAND = reg &0x20? 0xFF: 0x7F;
	int prgOR  = reg <<1;
	int chrOR  = reg <<4;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);	
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, reg &0x40? readPad: EMU->ReadPRG);
}

int getPRGBank (int bank) {
	if (reg &0x02) {
		int mask = reg &0x01? 3: 1;
		return MMC3::getPRGBank(0) &~mask | bank &mask;
	} else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int, int addr, int) {
	reg = addr;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
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

uint16_t mapperNum = 568;
} // namespace

MapperInfo MapperInfo_568 ={
	&mapperNum,
	_T("T-227"), /* 1993 Super HiK 60000-in-1 */
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
