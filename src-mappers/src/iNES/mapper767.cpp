#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg5000;
uint8_t		reg5080;
uint8_t		reg5180;
uint8_t		reg5300;
uint8_t		reg5388;
uint8_t		reg5400;
int		pa0809;

int	MAPINT	ppuReadNTSplit (int, int);
void	MAPINT	ppuWriteCHRSplit (int, int, int);

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	if (reg5000 ==0x50)
		EMU->SetPRG_RAM32(0x8, reg5400);
	else
		EMU->SetPRG_ROM32(0x8, 0);
	
	iNES_SetMirroring();
	
	if (reg5300 &0x80) {
		for (int bank =8; bank <12; bank++) EMU->SetPPUReadHandler (bank, ppuReadNTSplit);
		for (int bank =0; bank < 8; bank++) EMU->SetPPUWriteHandler(bank, ppuWriteCHRSplit);
	} else 
	for (int bank =0; bank <12; bank++) {
		EMU->SetPPUReadHandler (bank, EMU->ReadCHR);
		EMU->SetPPUWriteHandler(bank, EMU->WriteCHR);
		EMU->SetCHR_RAM8(0x0, reg5388);
	}
	for (int bank =0; bank <12; bank++) EMU->SetPPUReadHandlerDebug(bank, EMU->ReadCHRDebug);
}

int	MAPINT	readReg (int bank, int addr) {
	int result =0xFF;
	switch (addr) {
		case 0x000: result =reg5000; break;
		case 0x080: result =reg5080; break;
		case 0x0C0: result =0; EMU->SetIRQ(1); break;
		case 0x180: result =reg5180; break;
		case 0x300: result =reg5300; break;
		case 0x388: result =reg5388; break;
		case 0x400: result =reg5400; break;
	}
	return result;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	switch (addr) {
		case 0x000: reg5000 =val; break;
		case 0x080: reg5080 =val; break;
		case 0x180: reg5180 =val; break;
		case 0x300: reg5300 =val; break;
		case 0x388: reg5388 =val; break;
		case 0x400: reg5400 =val; break;
	}
	sync();
}

int	MAPINT	ppuReadNTSplit (int bank, int addr) {
	if (!(addr &0xFF)) {
		pa0809 =addr;
		EMU->SetCHR_RAM8(0x0, addr >>8 &0x03);
	}
	return EMU->ReadCHR(bank, addr);
}

void	MAPINT	ppuWriteCHRSplit (int bank, int addr, int val) {
	EMU->SetCHR_RAM8(0x0, reg5388);
	EMU->WriteCHR(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		reg5000 =0;
		reg5080 =0;
                reg5180 =0;
                reg5300 =0;
                reg5388 =0;
                reg5400 =0;
		pa0809 =0;
	}
	EMU->SetCPUReadHandler(0x5, readReg);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	sync();
}

void	MAPINT	cpuCycle (void) {
	return;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg5000);
	SAVELOAD_BYTE(stateMode, offset, data, reg5080);
	SAVELOAD_BYTE(stateMode, offset, data, reg5180);
	SAVELOAD_BYTE(stateMode, offset, data, reg5300);
	SAVELOAD_BYTE(stateMode, offset, data, reg5388);
	SAVELOAD_BYTE(stateMode, offset, data, reg5400);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =767;
} // namespace

MapperInfo MapperInfo_767 ={
	&mapperNum,
	_T("灵童"),
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
