#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC3.h"

namespace {
void	sync (void) {
	VRC3::syncPRG(0xFF, 0x00);
	VRC3::syncCHR(0xFF, 0x00);
	iNES_SetMirroring();
}

BOOL	MAPINT	load (void) {
	VRC3::load(sync);
	return TRUE;
}

uint16_t mapperNum =73;
} // namespace

MapperInfo MapperInfo_073 ={
	&mapperNum,
	_T("Konami VRC3"),
	COMPAT_FULL,
	load,
	VRC3::reset,
	NULL,
	VRC3::cpuCycle,
	NULL,
	VRC3::saveLoad,
	NULL,
	NULL
};