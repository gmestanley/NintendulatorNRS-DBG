#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
void	sync (void) {
	if (Latch::addr &0x08) {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
	iNES_SetCHR_Auto8(0x0, Latch::data);
	if (Latch::addr &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_AD(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =491;
} // namespace

MapperInfo MapperInfo_491 ={
	&mapperNum,
	_T("Sane Ting 5-in-1"),
	COMPAT_FULL,
	load,
	Latch::resetHard,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
