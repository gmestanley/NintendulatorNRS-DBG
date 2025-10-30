#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint16_t reg;

int MAPINT readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~3 | ROM->dipValue);
}

void sync (void) { 
	if (reg &0x20)
		MMC3::syncPRG(0x0F, reg <<1 &~0x0F);
	else
	if (ROM->INES2_SubMapper ==0 && (reg &6) ==6 ||
            ROM->INES2_SubMapper ==1 &&  reg &0x004 ||
	    ROM->INES2_SubMapper ==2 &&  reg &0x101 || 
	    ROM->INES2_SubMapper ==3 &&  reg &0x018)
		EMU->SetPRG_ROM32(0x8, reg >>1);
	else {
		EMU->SetPRG_ROM16(0x8, reg &0x1F);
		EMU->SetPRG_ROM16(0xC, reg &0x1F);
	}
	MMC3::syncCHR_ROM(0x7F, reg <<4 &~0x7F);
	MMC3::syncMirror();
	if (ROM->INES2_SubMapper >=2) for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, reg &0x80? readPad: EMU->ReadPRG);
}

void MAPINT writeReg (int, int addr, int) {
	reg =addr &0x1FF;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}
void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_WORD(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 339;
} // namespace

MapperInfo MapperInfo_339 ={
	&mapperNum,
	_T("K-3006/TL 8058"), // 0: K-3006, 1: unmarked, 2: TL 8058, 3: K-3091/GN-16
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