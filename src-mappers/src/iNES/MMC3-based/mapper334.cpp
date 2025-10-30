#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[2];

void sync (void) {
	EMU->SetPRG_ROM32(0x8, reg[0] >>1);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

int MAPINT readPad (int, int addr) {
	if (addr &2)
		return *EMU->OpenBus &0xFE | ROM->dipValue &0x01;
	else
		return *EMU->OpenBus;
}

void MAPINT writeReg (int, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =334;
} // namespace

MapperInfo MapperInfo_334 ={
	&mapperNum,
	_T("821202C"),
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