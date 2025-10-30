#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void sync (void) {
	int prg = Latch::addr >>2 &0x1F | Latch::addr >>3 &0x20 | Latch::addr >>4 &0x40;
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0x0, 0); 
	if (Latch::addr &0x080) {
		if (Latch::addr &0x001)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
		protectCHRRAM();
	} else {
		if (Latch::addr &0x800)
			EMU->SetPRG_ROM16(0x8, prg &~7 | Latch::data &7);
		else
			EMU->SetPRG_ROM16(0x8, prg);
		if (ROM->INES2_SubMapper ==2)
			EMU->SetPRG_ROM16(0xC, Latch::addr &0x200? (prg |7): 0);
		else
			EMU->SetPRG_ROM16(0xC, Latch::addr &0x200? (prg |7): (prg &~7));			
	}
	if (Latch::addr &0x002)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();	
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (Latch::addr &0x800)
		Latch::data = val;
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
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

uint16_t mapperNum = 375;
} // namespace

MapperInfo MapperInfo_375 = {
	&mapperNum,
	_T("135-in-1"),
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