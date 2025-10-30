#include "..\..\DLL\d_iNES.h"

namespace {
uint8_t reg[2];

void sync (void) {
	if (reg[1] &0x80) // First 2 MiB
		EMU->SetPRG_ROM16(0x8, reg[1] &0x70 | reg[0] &0x0F);
	else              // Optional extra chip, at end of PRG-ROM data
		EMU->SetPRG_ROM16(0x8, 0x80        | reg[0] &(ROM->INES_PRGSize -1) &0x0F);
	EMU->SetPRG_ROM16(0xC, reg[1] &0x7F);
	EMU->SetCHR_RAM8(0x0, 0);
	if (reg[0] &0x10)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (bank &6) {
		reg[1] =val &0x7F | bank <<5 &0x80;
	} else
		reg[0] =val;
	sync();
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0;
	sync();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =274;
} // namespace

MapperInfo MapperInfo_274 = {
	&mapperNum,
	_T("80013-B"),
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
