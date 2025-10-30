#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	if (reg &0x80) {
		int prg =reg >>1;
		EMU->SetPRG_ROM16(0x8, reg &0x20? prg &~1: prg);
		EMU->SetPRG_ROM16(0xC, reg &0x20? prg | 1: prg);
	} else
		MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR_ROM(0x1FF, 0x00);
	MMC3::syncMirror();
}

int getCHRBank (int bank) {
	return MMC3::getCHRBank(bank) | bank <<6 &0x100;
}

int MAPINT readProtection (int bank, int addr) {
	return *EMU->OpenBus |0x80;
}

void MAPINT writeReg (int bank, int addr, int val) { 
	if (~addr &1) {
		reg =val;
		sync();
	}
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, getCHRBank, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x5, readProtection);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =187;
} // namespace

MapperInfo MapperInfo_187 ={
	&mapperNum,
	_T("卡聖 A98402"),
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
