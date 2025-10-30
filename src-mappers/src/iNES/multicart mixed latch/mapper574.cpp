#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	Latch::setLockedBits(Latch::addr &0x80? 0xFC: 0x00, 0x00);
	if (Latch::addr &0x40)
		EMU->SetPRG_ROM32(0x8, Latch::addr >>2 &~0x07 | Latch::data &0x07);
	else
	if (Latch::addr &0x20)
		EMU->SetPRG_ROM32(0x8, Latch::addr >>2 &~0x03 | Latch::data &0x03);
	else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>2 &~0x03 | Latch::addr &0x03);
	if (Latch::addr &0x60) {
		if (Latch::data &0x10)
			EMU->Mirror_S1();
		else 
			EMU->Mirror_S0();
	} else
		if (Latch::data &0x10)
			EMU->Mirror_H();
		else 
			EMU->Mirror_V();
	EMU->SetCHR_RAM8(0, 0);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

uint16_t mapperNum = 574;
} // namespace

MapperInfo MapperInfo_574 = {
	&mapperNum,
	_T("FC-40"), /* (F-685) High K Battletoads 11-in-1 */
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};
