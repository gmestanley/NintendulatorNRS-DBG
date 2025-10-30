#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::addr <<3 | Latch::data &7);
	EMU->SetPRG_ROM16(0xC, Latch::addr <<3 | 7);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void MAPINT trapLatchWrite(int bank, int addr, int val) {
	Latch::write(bank, (addr ^ Latch::addr) &addr &0x10? addr: Latch::addr, val);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, trapLatchWrite);
}

uint16_t mapperNum = 363;
} // namespace

MapperInfo MapperInfo_363 = {
	&mapperNum,
	_T("5069"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};