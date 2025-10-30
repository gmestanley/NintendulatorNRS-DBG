#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;

void	sync (void) {
	if (reg &4) {
		EMU->SetPRG_ROM16(0x8, reg >>3 &0x1F | reg <<5 &0x20);
		EMU->SetPRG_ROM16(0xC, reg >>3 &0x1F | reg <<5 &0x20);
	} else
		EMU->SetPRG_ROM32(0x8, reg >>4 &0x0F | reg <<4 &0x10);
	MMC3::syncCHR_ROM(0xFF, reg <<7 &0x100);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (bank ==4) EMU->WriteAPU(bank, addr, val);
	if (addr &0x100) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
	for (int bank =0x4; bank <=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =474;
} // namespace

MapperInfo MapperInfo_474 = {
	&mapperNum,
	_T("NTDEC N625231"),
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