#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_VRC24.h"

namespace {
uint8_t irqEnabled;
uint16_t irqCounter;

void sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0xFF, 0x00);
	VRC24::syncMirror();
}

void MAPINT writeIRQ (int, int addr, int) {
	switch(addr &0xC) {
		// 900218
		case 0x08: irqEnabled =1; break;
		case 0x0C: irqEnabled =0; irqCounter =0; EMU->SetIRQ(1); break;
	}
}

BOOL MAPINT load (void) {
	VRC24::load(sync, false, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) irqEnabled =irqCounter =0;
	VRC24::reset(resetType);
	EMU->SetCPUWriteHandler(0xF, writeIRQ);
	EMU->SetIRQ(1);
}

void MAPINT cpuCycle (void) {
	if (irqEnabled && ++irqCounter &1024) EMU->SetIRQ(0);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, irqEnabled);
	SAVELOAD_WORD(stateMode, offset, data, irqCounter);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =524;
} // namespace

MapperInfo MapperInfo_524 = {
	&mapperNum,
	_T("900218"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
