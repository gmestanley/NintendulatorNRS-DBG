#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int bank =reg >>2 &0x08 | reg &0x06;
	MMC3::syncMirror();
	MMC3::syncPRG(0x1F, bank <<4);
	MMC3::syncCHR_ROM(0x7F, bank <<6);
}

void MAPINT writeReg (int, int, int val) {
	if (~reg &0x80) {
		reg =val;
		sync();
	}
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
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =267;
} // namespace

MapperInfo MapperInfo_267 ={
	&mapperNum,
	_T("晶太 EL861121C"),
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