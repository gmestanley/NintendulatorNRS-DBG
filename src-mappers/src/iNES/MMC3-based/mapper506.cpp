#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int prgAND =~reg &3? 0x0F: 0x1F; // 256 KiB if reg ==0, otherwise 128 KiB
	int chrAND = reg &3? 0x7F: 0xFF;
	int prgOR  = reg <<5 &0x20  | reg <<3 &0x10;
	int chrOR  = reg <<7 &0x100 |~reg <<7 &0x80;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg = addr &3;
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

uint16_t mapperNum = 506;
} // namespace

MapperInfo MapperInfo_506 = {
	&mapperNum,
	_T("GA-009"),
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
