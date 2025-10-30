#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int outerBank =reg >>3 &0x03 | reg >>4 &0x04;
	int chrAND =reg &0x80? 0xFF: 0x7F;
	if (reg &0x20) {
		int prgAND =reg &0x80? 0x1F: 0x0F;
		MMC3::syncPRG(prgAND, outerBank <<4 &~prgAND);
	} else
	if (reg &3)
		EMU->SetPRG_ROM32(0x8, outerBank <<2 | reg >>1 &0x3);
	else {
		EMU->SetPRG_ROM16(0x8, outerBank <<3 | reg     &0x7);
		EMU->SetPRG_ROM16(0xC, outerBank <<3 | reg     &0x7);
	}
	MMC3::syncCHR(chrAND, outerBank <<7 &~chrAND);
	MMC3::syncMirror();
}

void MAPINT writeReg (int, int addr, int) {
	reg =addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =322;
} // namespace

MapperInfo MapperInfo_322 = {
	&mapperNum,
	_T("6-15-C/K-3033"),
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
