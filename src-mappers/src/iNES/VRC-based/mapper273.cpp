#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_VRC24.h"

namespace {
uint8_t irqEnabled;
uint8_t irqCounter;
uint8_t irqPrescaler;
uint8_t irqMask;

void sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0xFF, 0x00);
	VRC24::syncMirror();
}

void MAPINT writeIRQ (int bank, int addr, int val) {
	switch (addr &8) {
		case 0:	irqCounter =val;
			EMU->SetIRQ(1);
			break;
		case 8:	irqEnabled =val;
			if (~irqEnabled &1) {
				irqPrescaler =0;
				irqMask =0x7F;
				EMU->SetIRQ(1);
			}
			break;
	}
}

BOOL MAPINT load (void) {
	VRC24::load(sync, false, 0x04, 0x08, NULL, true, 0);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	irqEnabled =0;
	irqCounter =0;
	irqPrescaler =0;
	irqMask =0;
	VRC24::reset(resetType);
	EMU->SetCPUWriteHandler(0xF, writeIRQ);
}

void MAPINT cpuCycle (void) {
	if (irqEnabled &1 && !(++irqPrescaler &irqMask)) {
		irqMask =0xFF;
		if (!++irqCounter)
			EMU->SetIRQ(0);
		else
			EMU->SetIRQ(1);
	}
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, irqEnabled);
	SAVELOAD_BYTE(stateMode, offset, data, irqCounter);
	SAVELOAD_BYTE(stateMode, offset, data, irqPrescaler);
	SAVELOAD_BYTE(stateMode, offset, data, irqMask);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =273;
} // namespace

MapperInfo MapperInfo_273 = {
	&mapperNum,
	_T("J-3?-C"),
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
