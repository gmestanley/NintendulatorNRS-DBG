#include "..\DLL\d_iNES.h"

namespace {
uint8_t reg;
uint8_t nt;

void sync() {
	EMU->SetPRG_RAM8 (0x6, 0);
	EMU->SetPRG_ROM32(0x8, 0);
	EMU->SetCHR_RAM4 (0x0, 0);
	EMU->SetCHR_RAM4 (0x4, reg &2 && !(reg &1 && nt ==3)? (nt +2): 1);
	iNES_SetMirroring();
}

void MAPINT writeReg (int, int, int val) {
	reg =val;
	sync();
}

int MAPINT trapNTRead (int bank, int addr) {
	if ((addr &0x0FF) ==0) {
		nt = addr >>8 &3;
		sync(); 
	}
	return EMU->ReadCHR(bank, addr);
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	nt =0;
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank = 0x8; bank <= 0xB; bank++) EMU->SetPPUReadHandler(bank, trapNTRead);
	sync();
}

uint16_t mapperNum =497;
} // namespace

MapperInfo MapperInfo_497 ={
	&mapperNum,
	_T("Subor LOGO"),
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