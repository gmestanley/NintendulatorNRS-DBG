#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;
bool		resetState;

void	sync (void) {
	if (resetState) {
		if ((reg &3) ==3)
			EMU->SetPRG_ROM32(0x8, reg <<2 &0xC | reg >>2 &0x3);
		else
			MMC3::syncPRG(0x0F, reg <<4 &0xF0);
		MMC3::syncCHR_ROM(0x7F, reg <<7 &0x180);
	} else {
		EMU->SetPRG_ROM32(0x8, reg >>4 &0x3 | 0x10);
		MMC3::syncCHR_ROM(0x7F, 0x200);
	}
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int, int addr, int val) {
	if (resetState)
		reg =addr &0xFF;
	else
		reg =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	resetState =resetType ==RESET_HARD? false: !resetState;
	reg =0;
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =475;
} // namespace

MapperInfo MapperInfo_475 = {
	&mapperNum,
	_T("820215-C-A2"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
