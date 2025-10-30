#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[4];

void sync (void) {
	int prgOR =reg[0] &0xF | reg[0] >>2 &0x10;
	if (reg[0] &0x80) {
		EMU->SetPRG_ROM16(0x8, reg[0] &0x20? prgOR &~1: prgOR);
		EMU->SetPRG_ROM16(0xC, reg[0] &0x20? prgOR | 1: prgOR);
	} else
		MMC3::syncPRG(0x1F, prgOR <<1 &~0x1F);
	MMC3::syncCHR_ROM(0xFF, reg[1] <<8); 
	MMC3::syncMirror();
}

int MAPINT readSolderPad (int bank, int addr) {
	return (addr &3) ==2? (ROM->dipValue &7 | *EMU->OpenBus *~7): *EMU->OpenBus;
}

void MAPINT writeReg (int bank, int addr, int val) { 
	reg[addr &3] =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, readSolderPad, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =115;
uint16_t mapperNum2 =248;
} // namespace

MapperInfo MapperInfo_115 ={
	&mapperNum,
	_T("卡聖 SFC-02B/-03/-004"),
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
MapperInfo MapperInfo_248 ={
	&mapperNum2,
	_T("卡聖 SFC-02B/-03/-004"),
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