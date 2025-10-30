#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
#define prgAND     0x0F
#define chrAND    (0xFF)
#define prgOR     (reg <<4 &0x20 | reg &0x10)
#define chrOR     (reg <<8)

uint8_t		reg;
FCPURead	readCart;

void	sync (void) {
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

int getPRGBank (int bank) {
	if (reg &0x04) {
		int mask = reg &0x08? 1: 3;
		return MMC3::getPRGBank(bank &1) &~mask | bank &mask;
	} else
		return MMC3::getPRGBank(bank);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg =addr &0xFF;
	sync();
}

int	MAPINT	trapCartRead (int bank, int addr) {
	if (~reg &0x04 && reg &0x08)
		return ROM->dipValue;
	else
		return readCart(bank, addr);
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);

	readCart =EMU->GetCPUReadHandler(0x8);
	for (int bank =0x8; bank<=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, trapCartRead);
		EMU->SetCPUReadHandlerDebug(bank, trapCartRead);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =443;
} // namespace

MapperInfo MapperInfo_443 = {
	&mapperNum,
	_T("NC3000M"),
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