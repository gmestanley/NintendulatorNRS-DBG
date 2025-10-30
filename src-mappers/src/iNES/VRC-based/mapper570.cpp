#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_VRC24.h"

namespace {
uint8_t reg;

void sync (void) {
	VRC24::syncPRG(0x0F, reg <<4);
	VRC24::syncCHR(reg &3? 0x0FF: 0x1FF, !!(reg &3)*0x200 | !!(reg &2)*0x100);
	VRC24::syncMirror();
	EMU->SetCHR_RAM8(0x0, 0);
}

void MAPINT writeReg (int, int addr, int) {
	reg = addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg = 0;
	VRC24::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

uint16_t mapperNum = 570;
} // namespace

MapperInfo MapperInfo_570 = {
	&mapperNum,
	_T("9052"), /* (9052 PCB) 4-in-1 */
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	NULL,
	NULL
};
