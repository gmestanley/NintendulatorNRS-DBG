#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int prgAND = reg &0x10? 0x1F: 0x0F;
	int chrAND = reg &0x10? 0xFF: 0x7F;
	int prgOR  = reg <<1;
	int chrOR  = reg <<4;
	if (reg &0x20)
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
	else
	if (reg &0x06)
		EMU->SetPRG_ROM32(0x8, reg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, reg);
		EMU->SetPRG_ROM16(0xC, reg);
	} 
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (~reg &0x20) {
		reg =addr &0xFF;
		sync();
	}
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 509;
} // namespace

MapperInfo MapperInfo_509 = {
	&mapperNum,
	_T("K-3022"), /* 32-in-1 高K 瑪琍兄弟大全 */
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