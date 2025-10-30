#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
void sync (void) {
	MMC3::syncWRAM(0);
	MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR(0xFF, 0x00);
	MMC3::syncMirror();
}

int getPRGPage (int bank) {
	int result =MMC3::getPRGBank(bank);
	return result &~0x0C | result >>1 &0x04 | result <<1 &0x08;
}

int getCHRPage (int bank) {
	int result =MMC3::getCHRBank(bank);
	return result &~0x22 | result >>4 &0x02 | result <<4 &0x20;
}

void MAPINT writeASIC (int bank, int addr, int val) {
	MMC3::write(bank, addr >>3, val);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGPage, getCHRPage, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	MMC3::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeASIC);
}

uint16_t mapperNum =325;
} // namespace

MapperInfo MapperInfo_325 = {
	&mapperNum,
	_T("Mali Splash Bomb"),
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
