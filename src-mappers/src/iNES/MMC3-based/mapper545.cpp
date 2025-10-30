#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0x7F, 0x00);
	MMC3::syncCHR(0x7F, reg <<7 | ~reg <<4 &0x40);
	MMC3::syncWRAM(0);

	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	int result = MMC3::getPRGBank(bank);
	if (reg &0x08 && ~result &0x10)
		result =          0x40 | result &0x0F;
	else
		result = reg <<4 &0x30 | result &0x0F;
	return result;
}


void MAPINT writeReg (int bank, int addr, int val) {
	if (addr &0x020 && addr &0x100) {
		reg = val;
		sync();
	} else
		MMC3::writeIRQEnable(bank, addr, val);
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) { 
	reg = 0;
	MMC3::reset(resetType); 
	EMU->SetCPUWriteHandler(0xF, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 545;
} // namespace

MapperInfo MapperInfo_545 = {
	&mapperNum,
	_T("ST-80"),
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