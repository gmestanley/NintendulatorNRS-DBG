#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0x3F, 0);
	if (reg &0x40)
		EMU->SetCHR_RAM8(0x00, 0x00);
	else
		MMC3::syncCHR_ROM(0x1FF, 0x000);
	EMU->Mirror_4();
}

int getCHRPage (int bank) {
	return MMC3::getCHRBank(bank) | reg <<5 <<(bank >>1) &0x100;
}

int MAPINT readSolderPad (int bank, int addr) {
	if (addr &0x100)
		return ROM->dipValue;
	else
		return EMU->ReadAPU(bank, addr);
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (bank ==0x4 && addr <0x020)
		EMU->WriteAPU(bank, addr, val);
	else
	if (addr &0x100) {
		reg =val &~3 | val <<1 &2 | val >>1 &1; // swap bits 0 and 1
		sync();
	}
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, getCHRPage, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x4, readSolderPad);
	for (int bank =0x4; bank <=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =262;
} // namespace

MapperInfo MapperInfo_262 ={
	&mapperNum,
	_T("Street Heroes"),
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
