#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data >>2);
	iNES_SetCHR_Auto8(0, Latch::data);
	if (ROM->INES2_SubMapper == 1) {
		if (Latch::data &0x80)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else
		iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, NULL);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	Latch::data =0x04; // ?? "Initial bank 1"
	sync();
}

uint16_t mapperNum =429;
} // namespace

MapperInfo MapperInfo_429 ={
	&mapperNum,
	_T("LIKO BBG-235-8-1B/Milowork FCFC1"),
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