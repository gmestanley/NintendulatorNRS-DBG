#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

extern	MapperInfo MapperInfo_012_1;	// in mapper006.cpp
extern	MapperInfo MapperInfo_SL5020B;

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR_ROM(0x1FF, 0x000);
	MMC3::syncMirror();
}

int getCHRPage (int bank) {
	return MMC3::getCHRBank(bank) | reg >>(bank &4) <<8;
}

int MAPINT readSolderPad (int bank, int addr) {
	return addr &0x100? ROM->dipValue: EMU->ReadAPU(bank, addr);
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (addr &0x100) {
		reg =val;
		sync();
	} else
		EMU->WriteAPU(bank, addr, val);
}

BOOL MAPINT load (void) {
	if (ROM->INES2_SubMapper ==1)
		memcpy(&MapperInfo_012, &MapperInfo_012_1, sizeof(MapperInfo));
	else
		memcpy(&MapperInfo_012, &MapperInfo_SL5020B, sizeof(MapperInfo));
	return MapperInfo_012.Load();
}

BOOL MAPINT loadSL5020B (void) {
	MMC3::load(sync, MMC3Type::NEC, NULL, getCHRPage, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x4, readSolderPad);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =12;
} // namespace

MapperInfo MapperInfo_012 ={
	&mapperNum,
	_T("哥德 SL-5020B/Front Fareast Magic Card 4M"),
	COMPAT_FULL,
	load,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

MapperInfo MapperInfo_SL5020B ={
	&mapperNum,
	_T("哥德 SL-5020B"),
	COMPAT_FULL,
	loadSL5020B,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
