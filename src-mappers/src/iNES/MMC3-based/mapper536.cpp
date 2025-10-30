#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[2];

void sync (void) {
	int prg = reg[0] &0x1F | reg[0] >>1 &0x20;
	if (reg[0] &0x80)
		MMC3::syncPRG(0x0F, prg <<2);
	else
	if (reg[0] &0x20) 
		EMU->SetPRG_ROM32(0x8, prg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg);
	}
	EMU->SetCHR_RAM8(0x0, 0);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[addr &1] = val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg[0] = 0x60;
	reg[1] = 0x00;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg[0]);
	SAVELOAD_BYTE(stateMode, offset, data, reg[1]);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 536;
} // namespace

MapperInfo MapperInfo_536 ={
	&mapperNum,
	_T("N42S-2"),
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
