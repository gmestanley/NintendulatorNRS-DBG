#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	Latch::setLockedBits(Latch::addr &0x200? ~0x1C: 0x00, 0x00);
	int prg = Latch::addr >>2 &0x1F;
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0x0, 0); 
	if (Latch::addr &0x200) {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg |7);
	} else {
		if (Latch::addr &0x001)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
	}
	if (Latch::addr &0x080) protectCHRRAM();
	if (Latch::addr &0x002)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, NULL, true);
	return TRUE;
}

uint16_t mapperNum = 579;
} // namespace

MapperInfo MapperInfo_579 = {
	&mapperNum,
	_T("T-215"), /* 超級金卡 9-in-1 128K */
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