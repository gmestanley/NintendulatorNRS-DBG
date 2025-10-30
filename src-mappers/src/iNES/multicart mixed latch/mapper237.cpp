#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
int	MAPINT	readPad (int bank, int addr) {
	return ROM->dipValue;
}

void	sync (void) {
	Latch::setLockedBits(Latch::addr &0x02? 0xFF: 0x00, Latch::addr &0x02? 0xF8: 0x00);
	int prg = Latch::data &0x1F | Latch::addr <<3 &0x20;
	if (Latch::data &0x80) {
		if (Latch::data &0x40)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
	} else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg | 7);
	}
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, Latch::addr &0x01? readPad: EMU->ReadPRG);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

uint16_t mapperNum = 237;
} // namespace

MapperInfo MapperInfo_237 = {
	&mapperNum,
	_T("Teletubbies 420-in-1"),
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