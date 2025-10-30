#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_VRC1.h"

namespace {
void	sync (void) {
	VRC1::syncPRG(0x1F, 0x00);
	VRC1::syncCHR(0x1F, 0x00);
	VRC1::syncMirror();
}

BOOL	MAPINT	load (void) {
	VRC1::load(sync);
	return TRUE;
}

uint16_t mapperNum =75;
} // namespace

MapperInfo MapperInfo_075 ={
	&mapperNum,
	_T("Konami VRC1"),
	COMPAT_FULL,
	load,
	VRC1::reset,
	NULL,
	NULL,
	NULL,
	VRC1::saveLoad,
	NULL,
	NULL
};