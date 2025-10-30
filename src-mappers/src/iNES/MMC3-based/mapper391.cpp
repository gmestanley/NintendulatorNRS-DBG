#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
#define	locked  !!(reg &0x80)
#define	prgOR     (reg <<4 &0x30)
#define	chrOR     (reg <<3 &0x080 | reg &0x100)
#define	prgAND    (reg &0x08? 0x0F: 0x1F)
#define	chrAND    (reg &0x40? 0x7F: 0xFF)

uint16_t	reg;

void	sync (void) {
	MMC3::syncWRAM(0);
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

int getPRGPage(int bank) {
	if (reg & 0x20) {
		int mask = reg & 0x04 ? 3 : 1;
		return MMC3::getPRGBank(bank &1) &~mask | bank &mask;
	}
	else
		return MMC3::getPRGBank(bank);
}


void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!locked) {
		reg =val | addr &0x100;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGPage, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_WORD(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =391;
} // namespace

MapperInfo MapperInfo_391 ={
	&mapperNum,
	_T("BS-110"),
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
