#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;
#define	locked  !!(reg &0x80)
#define	prgOR     (reg <<4 &0x30)
#define	chrOR     (reg <<3 &0x180)
#define	prgAND    (reg &0x08? 0x0F: 0x1F)
#define	chrAND    (reg &0x40? 0x7F: 0xFF)

void	sync (void) {
	MMC3::syncWRAM(0);
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

int getPRGBank(int bank) {
	if (reg & 0x04) {
		return MMC3::getPRGBank(bank &1) &~3 | bank &3;
	}
	else
		return MMC3::getPRGBank(bank);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!locked) {
		reg =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0x00;
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =441;
} // namespace

MapperInfo MapperInfo_441 ={
	&mapperNum,
	_T("850335C"),
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