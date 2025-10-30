#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x20) { // First chip: 256 KiB
		EMU->SetPRG_ROM16(0x8, Latch::addr >>1 &0x08 | Latch::data &0x07);
		EMU->SetPRG_ROM16(0xC, Latch::addr >>1 &0x08 | 0x07);
	} else { // Second chip: 128 KiB (boot)
		EMU->SetPRG_ROM16(0x8, 0x10 | Latch::addr >>1 &0x07);
		EMU->SetPRG_ROM16(0xC, 0x10 | Latch::addr >>1 &0x07);
	}
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::addr &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

void MAPINT trapLatchWrite (int bank, int addr, int val) { // The address bits are only writable on a falling edge of A3
	if (Latch::data &0x08 && ~val &0x08) Latch::addr = addr;
	Latch::data = val;
	sync();
}

void MAPINT reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, trapLatchWrite);
}

uint16_t MapperNum = 589;
} // namespace

MapperInfo MapperInfo_589 = {
	&MapperNum,
	_T("810706"),
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