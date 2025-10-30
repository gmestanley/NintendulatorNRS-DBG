#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_N118.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	N118::syncPRG(0x0F, 0x00);
	
	EMU->SetCHR_RAM8(0x0, 0);
	
	if (N118::reg[5] &0x20) // or in reverse 0x04/0x40
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	N118::load(sync);
	return TRUE;
}

uint16_t mapperNum =557;
} // namespace

MapperInfo MapperInfo_557 ={
	&mapperNum,
	_T("NTDEC 2718"),
	COMPAT_FULL,
	load,
	N118::reset,
	NULL,
	NULL,
	NULL,
	N118::saveLoad,
	NULL,
	NULL
};