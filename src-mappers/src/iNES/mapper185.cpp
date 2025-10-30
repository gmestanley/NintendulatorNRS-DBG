#include "..\DLL\d_iNES.h"
#include "..\Hardware\h_Latch.h"

namespace {
FCPURead readPPU;
uint8_t chrReads;

void sync (void) {
	EMU->SetPRG_ROM32(0x8, 0);
	if (ROM->INES2_SubMapper &4 && (ROM->INES2_SubMapper &3) !=(Latch::data &3))
		for (int bank =0; bank <8; bank++) EMU->SetCHR_OB1(bank);
	else
		EMU->SetCHR_ROM8(0, 0);
	iNES_SetMirroring();
}

int MAPINT interceptPPURead (int bank, int addr) {
	int result =readPPU(bank, addr);
	if ((addr &7) ==7) {
		if (ROM->INES2_SubMapper &4) {
			if (ROM->PRGROMCRC32 ==0xAA755715 && (ROM->INES2_SubMapper &3) !=(Latch::data &3))  // Mighty Bomb Jack (rev0) has a bodge resistor at PPU D0
				result |=1;
		} else
		if (chrReads <2) {
			result =0xFF;
			chrReads++;
		}
	}
	return result;
}

BOOL MAPINT load (void) {
	Latch::load(sync, Latch::busConflictAND);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	chrReads =0;
	Latch::reset(resetType);
	readPPU =EMU->GetCPUReadHandler(0x2);
	EMU->SetCPUReadHandler(0x2, interceptPPURead);
}

uint16_t mapperNum =185;
} // namespace

MapperInfo MapperInfo_185 = {
	&mapperNum,
	_T("Nintendo CNROM+Security"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};
