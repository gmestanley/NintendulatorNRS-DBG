#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

int MAPINT readPad (int, int) {
	return ROM->dipValue;
}

void sync (void) {
	int chrAND =reg &0x02? 0x7F: 0xFF;
	MMC3::syncPRG(0x0F, reg <<4 &~0x0F);
	MMC3::syncCHR(chrAND, reg <<7 &~chrAND);
	MMC3::syncMirror();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, reg &0x08? readPad: EMU->ReadPRG);
}

int getPRGPage (int bank) {
	if (reg &0x04)
		return MMC3::getPRGBank(0) &~3 | bank &3;
	else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int, int addr, int) {
	reg =addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::NEC, getPRGPage, NULL, NULL, writeReg); // 快打金卡终极挑战's "Revolution Hero" writes garbage to $6000-$7FFF, so don't use AX5202P
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

uint16_t mapperNum =344;
} // namespace

MapperInfo MapperInfo_344 = {
	&mapperNum,
	_T("GN-26"),
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