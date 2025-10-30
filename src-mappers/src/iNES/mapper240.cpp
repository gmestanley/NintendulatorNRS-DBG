#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM32(0x8, reg >>4);
	EMU->SetCHR_ROM8(0x0, reg &0xF);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank ==0x4) EMU->WriteAPU(bank, addr, val);
	if (addr &0x900) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load () {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	sync();
	for (int bank =0x4; bank <=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =240;
} // namespace

MapperInfo MapperInfo_240 = {
	&mapperNum,
	_T("聖火列傳/荊軻新傳"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};