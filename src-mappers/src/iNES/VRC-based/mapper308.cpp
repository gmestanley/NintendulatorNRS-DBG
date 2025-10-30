#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
uint8_t	IRQEnabled;
uint16_t IRQCounterLow;
uint8_t	IRQCounterHigh;

void	Sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0xFF, 0x00);
	VRC24::syncMirror();
}

void	MAPINT	WriteIRQ (int Bank, int Addr, int Val) {
	int Reg =((Addr &0x02)? 2: 0) | ((Addr &0x01)? 1: 0);
	switch (Reg) {
		case 0:	EMU->SetIRQ(1);
			IRQEnabled =0;
			IRQCounterLow =0;
			break;
		case 1:	IRQEnabled =1;
			break;
		case 3:	IRQCounterHigh =Val >>4;
			break;
	}
}

BOOL	MAPINT	Load (void) {
	VRC24::load(Sync, false, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType) {
	VRC24::reset(ResetType);
	EMU->SetCPUWriteHandler(0xF, WriteIRQ);
}

void	MAPINT	CPUCycle (void) {
	if (IRQEnabled) {
		if ((++IRQCounterLow &4095) ==2048) IRQCounterHigh--;
		if (!IRQCounterHigh && (IRQCounterLow &4095) <2048) EMU->SetIRQ(0);
	}
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, IRQEnabled);
	SAVELOAD_WORD(mode, offset, data, IRQCounterLow);
	SAVELOAD_BYTE(mode, offset, data, IRQCounterHigh);
	offset = VRC24::saveLoad(mode, offset, data);
	return offset;
}

uint16_t MapperNum =308;
} // namespace

MapperInfo MapperInfo_308 ={
	&MapperNum,
	_T("NTDEC 2131"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	CPUCycle,
	NULL,
	SaveLoad,
	NULL,
	NULL
};