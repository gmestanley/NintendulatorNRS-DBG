#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;
uint8_t inner;

void sync0 (void) {
	int prgAND = reg &0x20? 0x1F: 0x0F;
	int chrAND = reg &0x20? 0xFF: 0x7F;
	int prgOR  = reg <<4;
	int chrOR  = reg <<7;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR_ROM(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void sync1 (void) {
	int prgAND = (reg &0x1F) == 0x1F? 0x1F: 0x0F;
	int chrAND = (reg &0x1F) == 0x19? 0xFF: 0x7F;
	int prgOR  = reg <<4;
	int chrOR  = reg <<7;
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	if (reg &0x20)
		EMU->SetCHR_RAM8(0x0, 0);
	else
		MMC3::syncCHR_ROM(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void sync2 (void) {
	int prgAND = (reg &0x0E) == 0x0E? 0x1F: 0x0F;
	int chrAND = (reg &0x0F) == 0x03 || (reg &0x0F) == 0x0F? 0xFF: 0x7F;
	int prgOR  = reg <<4;
	int chrOR  = reg <<7;
	if (reg &0x20)
		EMU->SetPRG_ROM32(0x8, inner &3 | reg <<2 &~3);
	else
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
	if (reg &0x10) {
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) |0x400);
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(3) |0x400);
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) |0x400);
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7) |0x400);
	} else
		MMC3::syncCHR_ROM(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}


void MAPINT writeOuterBank (int bank, int addr, int val) {
	if (ROM->INES2_SubMapper == 2 && reg &0x30)
		inner = addr &0x100? (addr >>4): (val <<1 &2 | val >>4 &1);
	else
		reg = addr &0xFF;
	MMC3::sync();
}

BOOL MAPINT load (void) {
	MMC3::load(ROM->INES2_SubMapper == 2? sync2: ROM->INES2_SubMapper == 1? sync1: sync0, MMC3Type::AX5202P, NULL, NULL, NULL, writeOuterBank);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) { 
	reg = 0;
	inner = 3;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (ROM->INES2_SubMapper ==2) SAVELOAD_BYTE(stateMode, offset, data, inner);
	if (stateMode ==STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum = 480;
} // namespace

MapperInfo MapperInfo_480 ={
	&mapperNum,
	_T("480"),
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