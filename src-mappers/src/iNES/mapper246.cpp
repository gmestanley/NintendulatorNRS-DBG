#include	"..\DLL\d_iNES.h"

namespace {
uint8_t		reg[8];

void	sync (void) {
	EMU->SetPRG_RAM4(0x6, 0);
	for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, reg[bank]);
	for (int bank =0; bank <4; bank++) EMU->SetCHR_ROM2(bank*2, reg[bank +4]);
	iNES_SetMirroring();
}

int	MAPINT	readBank6 (int bank, int addr) {
	return addr &0x800? EMU->ReadPRG(bank, addr &0x7FF): *EMU->OpenBus;
}

void	MAPINT	writeBank6 (int bank, int addr, int val) {
	if (addr &0x800)
		EMU->WritePRG(bank, addr &0x7FF, val);
	else
	if ((addr &0xFE0) ==0x000) {
		reg[addr &7] =val;
		sync();
	}
}

int	MAPINT	readBankF (int bank, int addr) {
	int result;
	if ((addr &0xFE4) ==0xFE4) {
		EMU->SetPRG_ROM8(0xE, reg[3] | 0x10);
		result =EMU->ReadPRG(bank, addr);
		EMU->SetPRG_ROM8(0xE, reg[3]);
	} else
		result =EMU->ReadPRG(bank, addr);
	return result;
}

BOOL	MAPINT	load (void) {
	EMU->Set_SRAMSize(2048);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& r: reg) r =0xFF;
	sync();
	EMU->SetCPUReadHandler     (0x6, readBank6);
	EMU->SetCPUReadHandlerDebug(0x6, readBank6);
	EMU->SetCPUWriteHandler    (0x6, writeBank6);
	EMU->SetCPUReadHandler     (0xF, readBankF);
	EMU->SetCPUReadHandlerDebug(0xF, readBankF);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =246;
} // namespace

MapperInfo MapperInfo_246 ={
	&mapperNum,
	_T("G0151-1"),
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