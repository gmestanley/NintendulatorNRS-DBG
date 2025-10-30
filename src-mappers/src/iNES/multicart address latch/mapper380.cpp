#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
int MAPINT readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~0x0F | ROM->dipValue &0x0F);
}

void sync (void) {
	int prg = Latch::addr >>2 &0x1F;
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetCHR_RAM8(0x0, 0); 
	if (Latch::addr &0x200) {
		if (Latch::addr &0x001) {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		} else
			EMU->SetPRG_ROM32(0x8, prg >>1);
	} else {
		EMU->SetPRG_ROM16(0x8, prg);
		EMU->SetPRG_ROM16(0xC, prg | (ROM->INES2_SubMapper == 1 && Latch::addr &0x100? 15: 7));
	}
	if (Latch::addr &0x080) protectCHRRAM();
	if (Latch::addr &(ROM->INES2_SubMapper == 2? 0x040: 0x002))
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, ROM->INES2_SubMapper != 1 && Latch::addr &0x100? readPad: EMU->ReadPRG);
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	Latch::load(sync, NULL);
	MapperInfo_380.Description =ROM->INES2_SubMapper ==2? L"9441109 4K1": ROM->INES2_SubMapper ==1? L"KN-35A": L"970630C";
	return TRUE;
}

uint16_t mapperNum = 380;
} // namespace


MapperInfo MapperInfo_380 = {
	&mapperNum,
	_T("970630C/KN-35A"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_A,
	NULL,
	NULL
};