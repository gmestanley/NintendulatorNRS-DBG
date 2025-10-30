#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {	
	MMC3::syncPRG(0xFF, 0x00);
	MMC3::syncCHR_RAM(0x3F, 0);
	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	if (bank &2) 
		return MMC3::getPRGBank(bank) &0x3F;
	else
		return MMC3::getPRGBank(bank) &0x3F | MMC3::reg[0] &0xC0;
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, NULL);
	iNES_SetSRAM();
	return TRUE;
}

uint16_t mapperNum =513;
} // namespace

MapperInfo MapperInfo_513 ={
	&mapperNum,
	_T("SA-9602B"),
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