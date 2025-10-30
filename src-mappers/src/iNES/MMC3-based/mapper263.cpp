#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
void sync (void) {
	MMC3::syncPRG(0x3F, 0);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

void MAPINT write (int bank, int addr, int val) {
	MMC3::write(bank &~1, addr | bank &1, val &0xD8 | val <<3 &0x20 | val <<2 &0x04 | val >>4 &0x02 | val >>1 &0x01);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	MMC3::reset(resetType);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, write);
}

uint16_t mapperNum =263;
} // namespace

MapperInfo MapperInfo_263 = {
	&mapperNum,
	_T("S.M.I. NSM-xxx"),
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
