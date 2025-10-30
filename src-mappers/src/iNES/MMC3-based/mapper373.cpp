#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

#define prgAND ~reg[3] &0x3F
#define chrAND  0xFF >>(~reg[2] &0xF)
#define prgOR   reg[1] | reg[2] <<2 &0x300
#define chrOR   reg[0] | reg[2] <<4 &0xF00
#define gnrom   reg[2] &0x20
#define locked (reg[3] &0x40)

namespace {
uint8_t		index;
uint8_t		reg[4];

void	sync (void) {
	MMC3::syncWRAM(0);
	MMC3::syncPRG(prgAND, prgOR);
	MMC3::syncCHR_ROM(chrAND, chrOR);
	MMC3::syncMirror();
}

int getPRGBank(int bank) {
	if (gnrom)
		return MMC3::getPRGBank(bank &1) &~3 | bank &3;
	else
		return MMC3::getPRGBank(bank);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (!locked) {
		reg[index++ &3] =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	index =0;
	reg[0] =0x00;
	reg[1] =0x00;
	reg[2] =0x0F;
	reg[3] =0x00;
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =373;
} // namespace

MapperInfo MapperInfo_373 ={
	&mapperNum,
	_T("SFX-13"),
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