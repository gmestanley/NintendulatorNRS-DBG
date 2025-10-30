#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC1.h"

namespace {
uint8_t		reg;

void	sync (void) {
	if (reg &0x20)
		MMC1::syncPRG(0x07, reg &0x18);
	else
	if (reg &0x04)
		EMU->SetPRG_ROM32(0x8, reg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, reg);
		EMU->SetPRG_ROM16(0xC, reg);
	}
	MMC1::syncCHR(0x1F, reg <<2 &0x60);
	MMC1::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC1::load(sync, MMC1Type::MMC1A);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC1::reset(RESET_HARD);
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC1::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =498;
} // namespace

MapperInfo MapperInfo_498 = {
	&mapperNum,
	_T("K-3011"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC1::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};