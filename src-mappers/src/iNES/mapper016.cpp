#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FCG.h"

namespace {
EEPROM_I2C	*eeprom;

void	sync (void) {
	FCG::syncPRG(0x0F, 0x00);
	FCG::syncCHR(0xFF, 0x00);
	FCG::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	FCG::load(sync, ROM->INES2_SubMapper ==4? FCGType::FCG: ROM->INES2_SubMapper ==5? FCGType::LZ93D50: FCGType::Unknown, CART_BATTERY? new EEPROM_24C02(0, ROM->PRGRAMData): NULL);
	return TRUE;
}

uint16_t mapperNum =16;
} // namespace

MapperInfo MapperInfo_016 ={
	&mapperNum,
	_T("Bandai FCG"),
	COMPAT_FULL,
	load,
	FCG::reset,
	FCG::unload,
	FCG::cpuCycle,
	NULL,
	FCG::saveLoad,
	NULL,
	NULL
};
