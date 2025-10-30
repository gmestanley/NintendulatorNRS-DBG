#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_ROM4(0, reg &0xF);
	EMU->SetCHR_ROM4(4, reg >>4);
	iNES_SetMirroring();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0x00;
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	sync();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg); 
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =184;
} // namespace

MapperInfo MapperInfo_184 ={
	&mapperNum,
	_T("Sunsoft-1"),
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