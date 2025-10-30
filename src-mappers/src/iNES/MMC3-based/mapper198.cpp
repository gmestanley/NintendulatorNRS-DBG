#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
void sync (void) {
	EMU->SetPRG_RAM4(0x5, 2);
	MMC3::syncWRAM(0);
	MMC3::syncPRG(0x7F, 0x00);
	MMC3::syncCHR(0xFF, 0x00);
	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	int result =MMC3::getPRGBank(bank);
	return result &(result >=0x40? 0x4F: 0xFF);
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, NULL);
	return TRUE;
}

uint16_t mapperNum =198;
} // namespace

MapperInfo MapperInfo_198 ={
	&mapperNum,
	_T("BJ-0026"),
	COMPAT_FULL,
	load,
	MMC3::reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};