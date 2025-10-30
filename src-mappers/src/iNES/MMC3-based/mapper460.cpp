#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

int MAPINT readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~3 | ROM->dipValue &3);
}

void sync (void) {
	MMC3::syncPRG(0x0F, reg <<4);	

	if (reg &0x04) {
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0));
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(3));
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4));
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7));
	} else
		EMU->SetCHR_RAM8(0x0, 0);
	
	MMC3::syncMirror();
	
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUReadHandler(bank, ROM->INES2_SubMapper == 0 && reg &0x80 || ROM->INES2_SubMapper == 1 && reg &0x20? readPad: EMU->ReadPRG);
}

int getPRGBank (int bank) {
	if (reg &0x08) {
		int mask = reg &0x10? 3: 1;
		return MMC3::getPRGBank(bank &1) &~mask | bank &mask;
	} else
		return MMC3::getPRGBank(bank);
}

void MAPINT writeReg (int, int addr, int) {
	reg = addr &0xFF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::Sharp, getPRGBank, NULL, NULL, writeReg); // Must ignore stray 6xxx writes
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg = 0;
	MMC3::reset(resetType); 
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode == STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum = 460;
} // namespace

MapperInfo MapperInfo_460 ={
	&mapperNum,
	_T("FC-29-40/K-3101"),
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

