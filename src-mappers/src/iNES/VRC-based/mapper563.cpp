#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_VRC24.h"

namespace { // http://krzysiobal.com/carts/?action=view&id=118
uint8_t irqEnabled;
uint16_t irqCounter;
int	prevAddr;

void sync (void) {
	VRC24::syncPRG(0x1F, 0x00);
	VRC24::syncCHR_ROM(0xFF, 0x00);
	VRC24::syncMirror();
}

void MAPINT writeIRQ (int, int addr, int) {
	EMU->SetIRQ(1);
	switch(addr &0x1C) {
		case 0x0C: irqEnabled =0; break;
		case 0x08: irqEnabled =1; break;
		case 0x1C: irqCounter =0; break;
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

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (prevAddr &0x1000 && ~addr &0x1000 && !(++irqCounter &15) && irqEnabled) EMU->SetIRQ(0);
	prevAddr =addr;
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, irqEnabled);
	SAVELOAD_WORD(stateMode, offset, data, irqCounter);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =563;
} // namespace

MapperInfo MapperInfo_563 = {
	&mapperNum,
	_T("J-2020"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};
