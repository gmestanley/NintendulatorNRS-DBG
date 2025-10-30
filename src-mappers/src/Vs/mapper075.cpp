#include	"..\DLL\d_VS.h"
#include	"..\Hardware\h_VS.h"
#include	"..\Hardware\h_VRC1.h"

namespace {
void	sync (void) {
	VRC1::syncPRG(0x1F, 0x00);
	VRC1::syncCHR(0x1F, 0x00);
	EMU->Mirror_4();
}

BOOL	MAPINT	load (void) {
	VS::Load();
	VRC1::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	VS::Reset(resetType);
	VRC1::reset(resetType);
}

void	MAPINT	unload (void) {
	VS::Unload();
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VS::SaveLoad(stateMode, offset, data);
	offset =VRC1::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 75;
uint16_t mapperNum2 = 151;
} // namespace

MapperInfo MapperInfo_075 ={
	&mapperNum,
	_T("Konami VRC1"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	VS::CPUCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};

MapperInfo MapperInfo_151 ={
	&mapperNum2,
	_T("Konami VRC1"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	VS::CPUCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
