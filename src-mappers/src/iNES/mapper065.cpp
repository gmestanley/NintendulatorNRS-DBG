#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_H3001.h"

namespace {
void	sync (void) {
	H3001::syncPRG(0x1F, 0x00);
	H3001::syncCHR(0xFF, 0x00);
	H3001::syncMirror();
}

BOOL	MAPINT	load (void) {
	H3001::load(sync);
	return TRUE;
}

uint16_t mapperNum =65;
} // namespace

MapperInfo MapperInfo_065 ={
	&mapperNum,
	_T("Irem H-3001"),
	COMPAT_FULL,
	load,
	H3001::reset,
	NULL,
	H3001::cpuCycle,
	NULL,
	H3001::saveLoad,
	NULL,
	NULL
};