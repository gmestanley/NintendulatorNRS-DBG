#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data);
	EMU->SetPRG_ROM16(0xC, Latch::data);
	EMU->SetCHR_ROM8(0, Latch::data);
	if (Latch::data &0x08)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, Latch::busConflictAND);
	return TRUE;
}
uint16_t mapperNum =592;
} // namespace

MapperInfo MapperInfo_592 ={
	&mapperNum,
	_T("8-in-1 1991"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	Latch::saveLoad_D,
	NULL,
	NULL
};