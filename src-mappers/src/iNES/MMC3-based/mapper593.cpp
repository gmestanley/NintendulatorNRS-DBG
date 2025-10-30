#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
void	sync (void) {
	EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetCHR_ROM4(0x0, MMC3::reg[0]);
	EMU->SetCHR_ROM4(0x4, MMC3::reg[1]);
	EMU->SetPRG_ROM8(0x8, MMC3::reg[2]);
	EMU->SetPRG_ROM8(0xA, MMC3::reg[3]);
	EMU->SetPRG_ROM8(0xC, 0xFE);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	if (MMC3::reg[7] &0x01)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	MMC3::reg[bank &7] =val;
	if ((bank &7) ==6) {
		MMC3::write(0xC, 0, val -1);
		MMC3::write(0xC, 1, val);
		MMC3::write(0xE, 0, val);
		MMC3::write(0xE, val? 1: 0, val);
	}
	sync();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	MMC3::reset(resetType);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =593; // Originally mapper 4064
} // namespace

MapperInfo MapperInfo_593 = {
	&mapperNum,
	_T("PMMC3"),
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
