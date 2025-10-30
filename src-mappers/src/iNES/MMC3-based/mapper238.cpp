#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0xFF,0x00);
	MMC3::syncCHR_ROM(0xFF,0x00);
	MMC3::syncMirror();
}

int MAPINT readProtection (int bank, int addr) {
	static uint8_t lut[4] = { 0x00, 0x02, 0x02, 0x03 };
	return bank ==4 && addr <0x20? EMU->ReadAPU(bank, addr): lut[reg &3];
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (bank ==4 && addr <0x20)
		EMU->WriteAPU(bank, addr, val);
	else
		reg =val;
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}
void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	MMC3::reset(resetType);
	for (int bank =0x4; bank <=5; bank++) {
		EMU->SetCPUReadHandler(bank, readProtection);
		EMU->SetCPUWriteHandler(bank, writeReg);
	}
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 238;
} // namespace

MapperInfo MapperInfo_238 ={
	&mapperNum,
	_T("Sakano MMC3"),
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
