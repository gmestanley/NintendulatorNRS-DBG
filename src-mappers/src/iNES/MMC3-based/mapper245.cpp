#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t lastCHRBank;

void sync (void) {
	MMC3::syncMirror();
	MMC3::syncPRG(0x3F, MMC3::getCHRBank(lastCHRBank) &0x02? 0x40: 0x00);
	MMC3::syncWRAM(0);
	EMU->SetCHR_RAM8(0x0, 0x0);
}

int MAPINT interceptCHRRead (int bank, int addr) {
	int result =EMU->ReadCHR(bank, addr);
	bank &=3;
	if (lastCHRBank !=bank) {
		lastCHRBank =bank;
		sync();
	}
	return result;
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	MMC3::reset(resetType);
	for (int bank =0; bank <8; bank++) {
		EMU->SetPPUReadHandler(bank, interceptCHRRead);
		EMU->SetPPUReadHandlerDebug(bank, EMU->ReadCHRDebug);
	}
}

uint16_t mapperNum =245;
} // namespace

MapperInfo MapperInfo_245 = {
	&mapperNum,
	_T("外星 FS003"),
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