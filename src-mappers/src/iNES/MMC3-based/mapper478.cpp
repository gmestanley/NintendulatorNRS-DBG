#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg;
uint8_t		mode;

void	sync (void) {
	int prgAND =reg &8 && reg &4? 0x03: 0x0F;
	int chrAND =reg &8 && reg &4? 0x1F: 0x7F;
	MMC3::syncPRG(prgAND, reg <<2 &~prgAND);
	MMC3::syncCHR(chrAND, reg <<5 &~chrAND);
	MMC3::syncMirror();
}

void	sync1 (void) {
	MMC3::syncPRG(0x0F, reg <<3 &~0x0F);
	MMC3::syncCHR(0x7F, reg <<6 &~0x7F);
	MMC3::syncMirror();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (mode &0x60) {
		reg =addr &0xFF;
		mode =val;
		MMC3::sync();
	}
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	if (mode &0x80)
		MMC3::write(bank, addr, val);
	else {
		for (int index =0; index <6; index++) MMC3::reg[index] =MMC3::reg[index] &~0x18 | val <<3 &0x18;
		MMC3::sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(ROM->INES2_SubMapper ==1? sync1: sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	reg =0x00;
	mode =0xF0;
	MMC3::reset(RESET_HARD);
	if (ROM->INES2_SubMapper ==0) for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeASIC);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	SAVELOAD_BYTE(stateMode, offset, data, mode);
	if (stateMode ==STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum =478;
} // namespace

MapperInfo MapperInfo_478 = {
	&mapperNum,
	_T("WE7HGX"),
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