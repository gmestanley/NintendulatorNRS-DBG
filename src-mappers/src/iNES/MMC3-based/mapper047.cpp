#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncWRAM(0);
	MMC3::syncPRG(0xF, reg <<4);
	MMC3::syncCHR_ROM(0x7F, reg <<7);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (ROM->INES2_SubMapper ==0 || ~reg &0x80) {
		reg =val;
		sync();
	}
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::Sharp, NULL, NULL, NULL, writeReg);
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

uint16_t mapperNum =47;
} // namespace

MapperInfo MapperInfo_047 ={
	&mapperNum,
	_T("Nintendo NES-QJ"),
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