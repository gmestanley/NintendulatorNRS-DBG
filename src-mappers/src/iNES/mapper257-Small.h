﻿#include "..\DLL\d_iNES.h"

namespace DongdaSmall {
uint8_t	Mode, Index;
uint16_t LastNTAddr;
FCPURead _ReadCart;
FPPURead _ReadCHR, _ReadNT;
uint8_t prgSW;

static uint8_t BankLUT[128] = {
//        00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    10    11    12    13    14    15    16    17    18    19    1A    1B    1C    1D   1E    1F 	
	0x03, 0x13, 0x23, 0x33, 0x03, 0x13, 0x23, 0x33, 0x03, 0x13, 0x23, 0x33, 0x03, 0x13, 0x23, 0x33, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, // 00
	0x03, 0x13, 0x23, 0x33, 0x03, 0x13, 0x23, 0x33, 0x03, 0x13, 0x23, 0x33, 0x03, 0x13, 0x23, 0x33, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, // 20
	0x02, 0x12, 0x22, 0x32, 0x02, 0x12, 0x22, 0x32, 0x02, 0x12, 0x22, 0x32, 0x02, 0x12, 0x22, 0x32, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, 0x45, 0x67, // 40
	0x02, 0x12, 0x22, 0x32, 0x02, 0x12, 0x22, 0x32, 0x02, 0x12, 0x22, 0x32, 0x00, 0x10, 0x20, 0x30, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, 0x47, 0x67, // 60
};

static uint8_t ProtLUT[16] = {
	0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
};

void	Sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, BankLUT[Mode &0x7F] >> 4);
	EMU->SetPRG_ROM16(0xC, BankLUT[Mode &0x7F] &0xF);		
	if (prgSW &2)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
	EMU->SetCHR_RAM8(0x0, 0);
}

int	MAPINT	ReadCHR (int Bank, int Addr) {
	if (Mode &0x80) {
		// If the 1bpp mode is active, convert 1bpp to the normal 2bpp format.
		
		// Substitute A3 with NTRAM A0 (odd tile position)
		Addr &=~0x08;
		Addr |=(LastNTAddr &0x001)? 0x08: 0x00;
		
		// Substitute A12 with NTRAM A9 (scanline 128+)
		Bank &=~0x04;
		Bank |=(LastNTAddr &0x200)? 0x04: 0x00;
	}
	return _ReadCHR(Bank, Addr);
}

int	MAPINT	ReadNT (int Bank, int Addr) {
	if (Addr <0x3C0) LastNTAddr =Addr;
	return _ReadNT(Bank, Addr);
}

int	MAPINT	Read5 (int Bank, int Addr) {
	switch(Addr &0x700) {
		case 0x500: return EMU->tapeIn() *0x04;
		//case 0x500: return (*EMU->OpenBus &0xD8) | (ProtLUT[Index >>4]);
		default:    return *EMU->OpenBus;
	}
}

void	MAPINT	Write5 (int Bank, int Addr, int Val) {
	switch(Addr &0x700) {
		case 0x000:
			//if (Mode !=Val) EMU->DbgOut(L"%02X", Val);
			Mode =Val;
			break;
		case 0x100:
			EMU->tapeOut(!!(Val &0x01)); 
			if (prgSW ==Val) {
				prgSW =Val;
				Sync();
			}
			break;
		case 0x400: Index =Val; break;
	}
	Sync();
}

BOOL	MAPINT	Load (void) {
	return TRUE;
}

void	MAPINT	Reset (RESET_TYPE ResetType) {
	EMU->SetCPUReadHandler(0x5, Read5);
	EMU->SetCPUReadHandlerDebug(0x5, Read5);
	EMU->SetCPUWriteHandler(0x5, Write5);
	
	_ReadCHR =EMU->GetPPUReadHandler(0x0);
	_ReadNT  =EMU->GetPPUReadHandler(0x8);
	for (int i =0x0; i<=0x7; i++) {
		EMU->SetPPUReadHandler(i, ReadCHR);
		EMU->SetPPUReadHandlerDebug(i, _ReadCHR);
	}
	for (int i =0x8; i<=0xF; i++) {
		EMU->SetPPUReadHandler(i, ReadNT);
		EMU->SetPPUReadHandlerDebug(i, _ReadNT);
	}
	if (ResetType == RESET_HARD) {
		Index =0;
		Mode =0x0E;
	}
	Sync();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(mode, offset, data, Mode);
	SAVELOAD_BYTE(mode, offset, data, Index);
	if (mode == STATE_LOAD) Sync();
	return offset;
}

uint16_t MapperNum =257;

MapperInfo MapperInfo_DongdaSmall ={
	&MapperNum,
	_T("东达 PEC-586 (Small)"),
	COMPAT_FULL,
	Load,
	Reset,
	::Unload,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
} // namespace
