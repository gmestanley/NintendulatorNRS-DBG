#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FCG.h"

namespace {
void	sync (void) {
	FCG::syncPRG(0x1F, 0x00); // Should be 0F, but there are fan translations that use 512 KiB
	FCG::syncCHR(0xFF, 0x00);
	FCG::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	FCG::load(sync, FCGType::LZ93D50, CART_BATTERY? new EEPROM_24C01(0, ROM->PRGRAMData): NULL);
	return TRUE;
}

uint16_t mapperNum =159;
} // namespace

MapperInfo MapperInfo_159 ={
	&mapperNum,
	_T("Bandai FCG with 24C01 EEPROM"),
	COMPAT_FULL,
	load,
	FCG::reset,
	NULL,
	FCG::cpuCycle,
	NULL,
	FCG::saveLoad,
	NULL,
	NULL
};