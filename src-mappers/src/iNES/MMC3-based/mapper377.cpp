﻿#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t 	outerBank;

void	sync (void) {
	int bank =((outerBank &0x20) >>2) | (outerBank &0x06);
	
	MMC3::syncMirror();
	MMC3::syncPRG(0x0F, bank <<3);
	MMC3::syncCHR_ROM(0x7F, bank <<6);
}

void	MAPINT	writeOuterBank (int bank, int addr, int val) {
	if (~outerBank &0x80) {
		outerBank =val;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeOuterBank);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {	
	outerBank =0;
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =377;
} // namespace

MapperInfo MapperInfo_377 ={
	&mapperNum,
	_T("晶太 EL860947C"),
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