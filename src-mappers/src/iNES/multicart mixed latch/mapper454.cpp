﻿#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
#define cpuA14  !!(Latch::addr &0x001)
#define mirrorH !!(Latch::addr &0x002)
#define nrom    !!(Latch::addr &0x080)
#define unrom   !!(Latch::addr &0x100)
#define prg       (Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20) 

void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg &~cpuA14  &~(unrom*7) | (unrom*Latch::data));
	EMU->SetPRG_ROM16(0xC,(prg | cpuA14) &~(0x1F*!nrom*!unrom) |7*unrom);
	
	EMU->SetCHR_RAM8(0x0, 0);	
	if (nrom) protectCHRRAM();
	
	if (mirrorH)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (unrom)
		Latch::data =val &7;
	else
		Latch::write(bank, addr, val);
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE) {
	Latch::reset(RESET_HARD);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum =454;
} // namespace

MapperInfo MapperInfo_454 = {
	&mapperNum,
	_T("110-in-1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_AD,
	NULL,
	NULL
};