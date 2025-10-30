#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	Latch::setLockedBits(Latch::addr &0x20? 0x33: 0x00, 0x00);
	if (Latch::addr &0x10 && ROM->PRGROMSize >128*1024) // 128 KiB vs. 512 KiB check, /OE in the 512 KiB case
		for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetPRG_OB4(bank);
	else
	if (Latch::addr &0x02) { // UOROM
		EMU->SetPRG_ROM16(0x8, Latch::data &0x0F | Latch::addr <<3 &0x10);
		EMU->SetPRG_ROM16(0xC,              0x0F | Latch::addr <<3 &0x10);
	} else { // UNROM
		EMU->SetPRG_ROM16(0x8, Latch::data &0x07 | Latch::addr <<3 &0x18);
		EMU->SetPRG_ROM16(0xC,              0x07 | Latch::addr <<3 &0x18);
	}
	EMU->SetCHR_RAM8(0x0, 0);
	iNES_SetMirroring();
}

BOOL MAPINT load (void) {
	Latch::load(sync, Latch::busConflictAND, true);
	return TRUE;
}

uint16_t MapperNum = 573;
} // namespace

MapperInfo MapperInfo_573 = {
	&MapperNum,
	_T("5068"), // Super New HiK 3-in-1 - 高容量強卡三合一
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
