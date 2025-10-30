#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		extraVal;
uint8_t		extraAddr;
FCPUWrite	writeAPU;

void	sync (void) {
	if (extraVal &1) {
		if (extraVal &2)
			EMU->SetPRG_ROM32(0x8, extraVal >>3);
		else {
			EMU->SetPRG_ROM16(0x8, extraVal >>2);
			EMU->SetPRG_ROM16(0xC, extraVal >>2);
		}
		MMC3::syncCHR(0xFF, 0x00);
	} else {
		int prgAND =0x1F;
		int chrAND =extraAddr &2? 0x7F: 0xFF;
		int prgOR =extraVal >>1;
		int chrOR =extraAddr <<6;
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
		MMC3::syncCHR(chrAND, chrOR &~chrAND);
	}
	MMC3::syncMirror();
}

void	MAPINT	interceptAPUWrite (int bank, int addr, int val) {
	if (bank ==4) writeAPU(bank, addr, val);
	if (addr &0x100) {
		extraVal =val;
		extraAddr =addr &0xFF;
		sync();
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	extraVal =0;
	extraAddr =0;
	MMC3::reset(resetType);	
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, interceptAPUWrite);
	EMU->SetCPUWriteHandler(0x5, interceptAPUWrite);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, extraVal);
	SAVELOAD_BYTE(stateMode, offset, data, extraAddr);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =455;
} // namespace

MapperInfo MapperInfo_455 ={
	&mapperNum,
	_T("N625836"),
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