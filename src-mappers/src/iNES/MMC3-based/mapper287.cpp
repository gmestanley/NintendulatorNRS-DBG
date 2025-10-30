#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	if (reg &0x04 && ROM->PRGROMSize <1024*1024 && ROM->dipValue) // On cartridges with 512 KiB PRG-ROM, PRG A19 connects PRG /CE if configured such via solder pad
		for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
	else
	if (reg &0x08)
		EMU->SetPRG_ROM32(0x8, reg >>4 &3 | reg <<2 &~3);
	else
		MMC3::syncPRG(0x0F, reg <<4 &~0x0F);
	MMC3::syncCHR_ROM(0x7F, reg <<7 &~0x7F);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
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
	reg =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =287;
} // namespace

MapperInfo MapperInfo_287 = {
	&mapperNum,
	_T("811120-C/810849-C"),
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
