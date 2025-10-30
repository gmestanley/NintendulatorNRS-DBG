#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

int MAPINT readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~3 | ROM->dipValue &3);
}

void sync (void) {
	int prgAND = 0x0F;
	int chrAND = 0xFF;
	int prgOR  = reg <<4;
	int chrOR  = reg <<7;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, reg &0x08? readPad: EMU->ReadPRG);
}

int getPRGBank (int bank) {
	if (reg &0x04) {
		int mask = reg &0x02? 1: 3;
		return MMC3::getPRGBank(bank &1) &~mask | bank &mask;
	} else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int , int addr, int ) {
	reg =addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg = 0;
	MMC3::reset(RESET_HARD);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 511;
} // namespace

MapperInfo MapperInfo_511 = {
	&mapperNum,
	_T("1n4148"), /* 260-in-1 Game Screen Selectable */
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
