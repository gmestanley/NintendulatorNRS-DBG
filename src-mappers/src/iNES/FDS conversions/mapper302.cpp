#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"
#include	"..\..\Hardware\Sound\s_FDS.h"

namespace {
void	Sync (void) {
	EMU->SetPRG_ROM8 (0xA, 0x0D);
	EMU->SetPRG_ROM16(0xC, 0x07);
	EMU->SetCHR_RAM8(0, 0);
	if (VRC24::mirroring &1)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

int	MAPINT	Read (int Bank, int Addr) {
	int index= ((Bank -6) <<1) | ((Addr &0x800)? 1: 0);
	index ^=4;
	return ROM->PRGROMData[(VRC24::chr[index] <<11) | (Addr &0x7FF)];
	
}

int	MAPINT	read4 (int bank, int addr) {
	return addr <0x20? EMU->ReadAPU(bank, addr): FDSsound::Read(bank <<12 |addr);
}

void	MAPINT	write4 (int bank, int addr, int val) {
	if (addr <0x20)
		EMU->WriteAPU(bank, addr, val);
	else
		FDSsound::Write(bank <<12 | addr, val);
}

BOOL	MAPINT	Load (void) {
	VRC24::load(Sync, false, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	VRC24::reset(ResetType);
	FDSsound::Reset(ResetType);
	EMU->SetCPUReadHandler(0x4, read4);
	EMU->SetCPUWriteHandler(0x4, write4);
	for (int i =0x6; i<=0x9; i++) EMU->SetCPUReadHandler(i, Read);
	for (int i =0x6; i<=0x9; i++) EMU->SetCPUReadHandlerDebug(i, Read);
}

static int MAPINT MapperSnd (int Cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(Cycles): 0;
}

uint16_t MapperNum =302;
} // namespace

MapperInfo MapperInfo_302 = {
	&MapperNum,
	_T("Kaiser KS-7057"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	VRC24::saveLoad,
	MapperSnd,
	NULL
};