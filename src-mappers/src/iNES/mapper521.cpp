#include "..\DLL\d_iNES.h"
namespace {
uint8_t prg;

void sync (void) {
	EMU->SetPRG_ROM16(0x8, prg);
	EMU->SetPRG_ROM16(0xC, ROM->INES_PRGSize -1);
	EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

void MAPINT write (int, int addr, int val) {
	if (addr &0x20 && ~addr &0x10) {
		prg =val;
		sync();
	}
}

void MAPINT reset (RESET_TYPE resetType) {
	EMU->SetCPUWriteHandler(0x5, write);
	sync();
}

uint16_t mapperNum =521;
} // namespace

MapperInfo MapperInfo_521 = {
	&mapperNum,
	_T("Dreamtech 01"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};