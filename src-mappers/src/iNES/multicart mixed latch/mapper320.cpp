#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	if (Latch::addr &0x10) {
		EMU->SetPRG_ROM16(0x8, Latch::addr <<3 | Latch::data &0x07);
		EMU->SetPRG_ROM16(0xC, Latch::addr <<3 | 0x07);
	} else {
		EMU->SetPRG_ROM16(0x8, Latch::addr <<3 | Latch::data &0x0F);
		EMU->SetPRG_ROM16(0xC, Latch::addr <<3 | 0x0F);
	}
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

void MAPINT trapLatchWrite(int bank, int addr, int val) {
	Latch::write(bank, (addr ^ Latch::addr) &addr &0x20? addr: Latch::addr, val);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, trapLatchWrite);
}

uint16_t mapperNum = 320;
} // namespace

MapperInfo MapperInfo_320 ={
	&mapperNum,
	_T("830425C-4391T"),
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