#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int prgAND = reg &0x04? 0x0F: 0x1F;
	int chrAND = reg &0x02? 0x7F: 0xFF;
	int prgOR = reg <<4 &0x10 | reg <<2 &0x20 | reg <<1 &0xC0;
	int chrOR = reg <<7 &0x80 | reg <<4 &0x700;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	if (reg &0x80)
		MMC3::syncMirrorA17();
	else
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
	reg = 0;
	MMC3::reset(RESET_HARD);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 566;
} // namespace

MapperInfo MapperInfo_566 = {
	&mapperNum,
	_T("ET-149"),
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