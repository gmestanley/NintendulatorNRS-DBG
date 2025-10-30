#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	Latch::setLockedBits(0x100, 0x00);
	EMU->SetCHR_RAM8(0x0, 0);
	if (Latch::addr &0x100) {
		if (ROM->INES2_SubMapper == 1) {
			EMU->SetPRG_ROM16(0x8, Latch::addr >>2 &7 |0x20);
			EMU->SetPRG_ROM16(0xC,                     0x27);
		} else {
			EMU->SetPRG_ROM16(0x8, 0x20 | Latch::data &7);
			EMU->SetPRG_ROM16(0xC, 0x27);
		}
	} else {
		if (Latch::addr &0x080) {
			if (Latch::addr &0x001)
				EMU->SetPRG_ROM32(0x8, Latch::addr >>3 &0x0F);
			else {
				EMU->SetPRG_ROM16(0x8, Latch::addr >>2 &0x1F);
				EMU->SetPRG_ROM16(0xC, Latch::addr >>2 &0x1F);
			}
			protectCHRRAM();
		} else {
			EMU->SetPRG_ROM16(0x8, Latch::addr >>2);
			EMU->SetPRG_ROM16(0xC, 0);
		}		
	}
	if (Latch::addr &0002)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL MAPINT load (void) {
	Latch::load(sync, Latch::busConflictAND, true);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD)
		Latch::reset(RESET_HARD);
	else {
		Latch::reset(RESET_SOFT);
		Latch::addr = Latch::addr &0x100 ^0x100;
		sync();
	}
}

uint16_t mapperNum = 280;
} // namespace

MapperInfo MapperInfo_280 = {
	&mapperNum,
	_T("K-3017"),
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