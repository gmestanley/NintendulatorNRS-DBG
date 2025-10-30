#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void sync (void) {
	int prgAND =~MMC3::wramControl &6? 0x0F: 0x1F;
	int chrAND =~MMC3::wramControl &6? 0x7F: 0xFF;
	MMC3::syncPRG(prgAND, MMC3::wramControl <<4 &~prgAND);
	MMC3::syncCHR_ROM(chrAND, MMC3::wramControl <<7 &~chrAND);
	MMC3::syncWRAM(0);
	MMC3::syncMirror();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	MMC3::reset(RESET_HARD);
}

uint16_t mapperNum =44;
} // namespace

MapperInfo MapperInfo_044 = {
	&mapperNum,
	_T("Super HiK 7-in-1 (MMC3)"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	NULL,
	NULL
};