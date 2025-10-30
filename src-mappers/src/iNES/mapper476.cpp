#include	"..\DLL\d_iNES.h"
namespace {
uint16_t	reg[4];

void	sync (void) {
	if (reg[2] &4)		
		EMU->SetPRG_ROM32(0x8, reg[0]);
	else {
		EMU->SetPRG_ROM16(0x8, reg[0]);
		EMU->SetPRG_ROM16(0xC, reg[0]);
	}
	EMU->SetCHR_RAM8(0, 0);
	if (reg[2] &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int, int addr, int val) {
	reg[addr >>8 &3] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE) {
	for (auto& r: reg) r =0;
	sync();
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& r: reg) SAVELOAD_WORD(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =476;
} // namespace

MapperInfo MapperInfo_476 = {
	&mapperNum,
	_T("Croaky Karaoke"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};