#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

int MAPINT readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~0xF | ROM->dipValue &0xF);
}

void sync (void) {
	if (reg &0x20)
		EMU->SetPRG_ROM32(0x8, reg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, reg);
		EMU->SetPRG_ROM16(0xC, reg);
	}
	MMC3::syncCHR_ROM(0xFF, reg <<4 &0x100);
	MMC3::syncMirror();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, reg &0x80? readPad: EMU->ReadPRG);
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg = 0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =490;
} // namespace

MapperInfo MapperInfo_490 = {
	&mapperNum,
	_T("K-3101"),
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