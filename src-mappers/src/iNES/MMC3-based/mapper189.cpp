#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	EMU->SetPRG_ROM32(0x8, reg | reg >>4);
	MMC3::syncCHR_ROM(0xFF, 0);
	MMC3::syncMirror();
}

void MAPINT write45 (int bank, int addr, int val) {
	if (addr &0x100) {
		reg =val;
		MMC3::sync();
	} else
	if (bank ==0x4 && addr <0x020)
		EMU->WriteAPU(bank, addr, val);
}

void MAPINT write67 (int bank, int addr, int val) {
	reg =val;
	MMC3::sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, write67);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =3;
	MMC3::reset(resetType);
	for (int bank =0x4; bank <=0x5; bank++) EMU->SetCPUWriteHandler(bank, write45);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =189;
} // namespace

MapperInfo MapperInfo_189 ={
	&mapperNum,
	_T("TXC 01-22017-000/01-22018-400/FC-001EM(C)/K-1069A"),
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
