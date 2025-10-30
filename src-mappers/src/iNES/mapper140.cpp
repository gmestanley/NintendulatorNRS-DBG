#include "..\DLL\d_iNES.h"
#include "..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data >>4);
	iNES_SetCHR_Auto8(0x0, Latch::data &0xF);
	iNES_SetMirroring();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	Latch::reset(resetType);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, Latch::write);
}

uint16_t mapperNum =140;
} // namespace

MapperInfo MapperInfo_140 = {
	&mapperNum,
	_T("Jaleco GNROM"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
