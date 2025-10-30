#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[2];

void sync (void) {
	int prg =reg[0] &0x07 | reg[0] >>3 &0x08 | reg[1] <<4 &0x10;
	if (reg[0] &0x80) {
		if (reg[0] &0x20)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
	} else
		MMC3::syncPRG(0x0F, prg <<1 &~0x0F);
	MMC3::syncCHR(0x7F, reg[0] <<1 &0x80 | reg[1] <<8 &0x100);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

BOOL MAPINT load(void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =376;
} // namespace

MapperInfo MapperInfo_376 ={
	&mapperNum,
	_T("晶太 YY841155C, Realtec 9056"),
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
