#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	int prgAND = reg &2 && reg &4? 0x1F: 0x0F;
	int chrAND = reg &2 && reg &4? 0xFF: 0x7F;
	int prgOR  = reg <<4;
	int chrOR  = reg <<7;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (addr &2) {
		reg =val;
		sync();
	} else
		MMC3::write(bank, addr, val);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg = 0;
	MMC3::reset(RESET_HARD);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 579;
} // namespace

MapperInfo MapperInfo_578 = {
	&mapperNum,
	_T("910610"),
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