#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	EMU->SetCHR_RAM8(0, 0);
	int prg = Latch::addr >>1 &0x20 | Latch::addr &0x1F;
	if (Latch::addr &0x20) {
		if (Latch::addr &0x01) {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		} else
			EMU->SetPRG_ROM32(0x8, prg >>1);
		protectCHRRAM();
	} else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg | 7);
	}
	if (Latch::addr &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void MAPINT trapLatchWrite (int bank, int addr, int val) { // The upper bits are only writable on a falling edge of A5
	if (Latch::addr &0x20 && ~addr &0x20) Latch::setLockedBits(0x00, 0x00);
	Latch::write(bank, addr, val);
	Latch::setLockedBits(0xC0, 0x00);	
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, trapLatchWrite);
}

uint16_t mapperNum = 581;
} // namespace

MapperInfo MapperInfo_581 = {
	&mapperNum,
	_T("ET-82"), /* 東方巨龍 80-in-1 */
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AL,
	NULL,
	NULL
};