#include "..\DLL\d_iNES.h"
#include "..\Hardware\h_Latch.h"

namespace {
uint8_t chr;

int MAPINT trapNTRead (int bank, int addr) {
	if (addr <0x3C0) { // CHR A12 is CIRAM A10 during reads from PPU $0000-$0FFF
		uint8_t newCHR = bank >>(Latch::data >>6 &1) &1;
		if (newCHR != chr) EMU->SetCHR_RAM4(0x0, chr = newCHR);
	}
	return EMU->ReadCHR(bank, addr);
}

void MAPINT trapCHRWrite (int bank, int addr, int val) {
	EMU->SetCHR_RAM8(0x0, 0); // CHR A12 is always 0 during writes to PPU $0000-$0FFF
	EMU->WriteCHR(bank, addr, val);
}

void sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8, Latch::data);
	EMU->SetCHR_RAM4(0x0, Latch::data &0x80? chr: 0);
	EMU->SetCHR_RAM4(0x4, 1);
	if (Latch::data &0x40)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	for (int bank =0; bank <= 3; bank++) EMU->SetPPUWriteHandler    (bank, Latch::data &0x80? trapCHRWrite: EMU->WriteCHR);
	for (int bank =8; bank <=11; bank++) EMU->SetPPUReadHandler     (bank, Latch::data &0x80? trapNTRead: EMU->ReadCHR);
	for (int bank =8; bank <=11; bank++) EMU->SetPPUReadHandlerDebug(bank, EMU->ReadCHR);
}

BOOL MAPINT load (void) { 
	iNES_SetSRAM(); 
	Latch::load(sync, NULL);
	return TRUE;
}

uint16_t mapperNum =514;
} // namespace

MapperInfo MapperInfo_514 = {
	&mapperNum,
	_T("小霸王 卡拉 OK"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};