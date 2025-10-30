#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	if (ROM->INES2_SubMapper ==3)
		EMU->SetPRG_ROM32(0x8, reg >>4);
	else
		MMC3::syncPRG(0x3F, 0);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

int MAPINT readSolderPad (int, int) {
	return ROM->dipValue;
}

void MAPINT writeReg (int, int, int val) {
	reg =val;
	sync();
}

void MAPINT writeASIC (int bank, int addr, int val) {
	switch(ROM->INES2_SubMapper) {
		default: addr =!!(addr &0xE) ^(addr &1); break;
		case 1: addr >>=1; break;
		case 2: addr >>=2; break;
		case 3: addr >>=1; break;
	}
	MMC3::write(bank, addr, val);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x5, readSolderPad);
	if (ROM->INES2_SubMapper ==3) for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeASIC);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (ROM->INES2_SubMapper ==3) SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =196;
} // namespace

MapperInfo MapperInfo_196 ={
	&mapperNum,
	_T("MRCM UT1374"),
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
