#include "..\DLL\d_iNES.h"

namespace {
uint8_t reg[4];

void sync (void) {
	EMU->SetPRG_ROM16(0x8, reg[0] &7 | reg[1] <<3);
	EMU->SetPRG_ROM16(0xC, reg[2] &7 | reg[3] <<3);
	iNES_SetCHR_Auto8(0x0, 0);
	iNES_SetMirroring();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[bank >>1 &3] = val;
	sync();
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& c: reg) c = 0xFF;
	sync();
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 582;
} // namespace

MapperInfo MapperInfo_582 = {
	&mapperNum,
	_T("A9778"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};