#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_VRC24.h"

namespace {
uint8_t reg;

void sync (void) {
	int prgAND =reg &0x10? 0x1F: 0x0F;
	int chrAND =reg &0x10? 0xFF: 0x7F;
	int prgOR = reg <<1;
	int chrOR = reg <<4;
	if (reg &0x20) 
		VRC24::syncPRG(prgAND, prgOR &~prgAND);
	else {
		if (reg &0x6)
			EMU->SetPRG_ROM32(0x8, reg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, reg);
			EMU->SetPRG_ROM16(0xC, reg);
		}
	}
	VRC24::syncCHR(chrAND, chrOR &~chrAND);
	VRC24::syncMirror();
	VRC24::A0 =reg &0x10? 8: 4;
	VRC24::A1 =reg &0x10? 4: 8;
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (VRC24::wramEnable && ~reg &1) {
		reg = addr &0xFF;
		sync();
	}
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	VRC24::load(sync, true, 0x04, 0x08, NULL, true, 0);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg = 0;
	VRC24::reset(RESET_HARD);
	for (int bank = 0x6; bank <= 0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = VRC24::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =571;
} // namespace

MapperInfo MapperInfo_571 = {
	&mapperNum,
	_T("JC-011"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	VRC24::cpuCycle,
	NULL,
	saveLoad,
	NULL,
	NULL
};
