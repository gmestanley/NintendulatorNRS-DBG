#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	Latch::setLockedBits(0xFF, 0x00);	
	EMU->SetCHR_RAM8(0, 0);
	int prg = Latch::addr &~0xF | Latch::addr <<3 &0x08;
	if (Latch::addr &0x20) {
		EMU->SetPRG_ROM32(0x8, prg >>1 &~7 | Latch::data &7);
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else {
		if (Latch::addr &0x10 && Latch::addr &0x08) {
			EMU->SetPRG_ROM16(0x8, prg | Latch::data &3);
			EMU->SetPRG_ROM16(0xC, prg |              3);
		} else {
			EMU->SetPRG_ROM16(0x8, prg &~7 | Latch::data &7);
			EMU->SetPRG_ROM16(0xC, prg     |              7);
		}
		if (Latch::addr &0x04)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
}

void MAPINT trapLatchWrite (int bank, int addr, int val) { // Address bits only writable on a rising edge of A7
	if (~Latch::addr &0x80 && addr &0x80) Latch::setLockedBits(0x00, 0x00);
	Latch::write(bank, addr, val);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, trapLatchWrite);
}

uint16_t mapperNum = 583;
} // namespace

MapperInfo MapperInfo_583 = {
	&mapperNum,
	_T("8203"), /* (DY-116) Selling Card 7-in-1 */
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