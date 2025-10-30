#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_VRC24.h"

namespace {
struct {
	uint8_t enabled;
	uint8_t counter;
	uint8_t prescaler;
	uint8_t mask;
} irq;

void sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0xFF, 0x00);
	VRC24::syncMirror();
}

void MAPINT writeIRQ (int bank, int addr, int val) {
	switch (addr &0xC) {
		case 0:	irq.counter =val;
			irq.prescaler =0; // ??? Necessary to prevent Stage 2 from shaking
			EMU->SetIRQ(1);
			break;
		case 4:	irq.enabled =val;
			break;
	}
}

BOOL MAPINT load (void) {
	VRC24::load(sync, false, 0x08, 0x04, NULL, true, 0);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	irq.enabled =0;
	irq.counter =0;
	irq.prescaler =0;
	irq.mask =0;
	VRC24::reset(resetType);
	EMU->SetCPUWriteHandler(0xF, writeIRQ);
}

void MAPINT cpuCycle (void) {
	if (irq.enabled &1) {
		irq.prescaler++;
		if (irq.prescaler == 64) EMU->SetIRQ(!!++irq.counter);
		if (irq.prescaler ==112) irq.prescaler =0;
	} else {
		irq.prescaler =0;
		EMU->SetIRQ(1);
	}
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, irq.enabled);
	SAVELOAD_BYTE(stateMode, offset, data, irq.counter);
	SAVELOAD_BYTE(stateMode, offset, data, irq.prescaler);
	SAVELOAD_BYTE(stateMode, offset, data, irq.mask);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =565;
} // namespace

MapperInfo MapperInfo_565 = {
	&mapperNum,
	_T("J-33-C"),
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
