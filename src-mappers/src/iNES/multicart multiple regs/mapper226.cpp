#include "..\..\DLL\d_iNES.h"

namespace {
uint8_t reg[2];

void sync (void) {
	uint8_t outerBank =reg[0] >>7 | reg[1] <<1 &0x02;
	if (ROM->INES_PRGSize ==1536/16 && outerBank >0) outerBank--; // Mapper 226 with 1536 KiB: Outer bank order 0 0 1 2
	if (reg[0] &0x20) { // NROM-128
		EMU->SetPRG_ROM16(0x8, outerBank <<5 | reg[0] &0x1F);
		EMU->SetPRG_ROM16(0xC, outerBank <<5 | reg[0] &0x1F);
	} else // NROM-256
		EMU->SetPRG_ROM32(0x8,(outerBank <<5 | reg[0] &0x1E) >>1);

	EMU->SetCHR_RAM8(0, 0);
	if (reg[1] &0x02) protectCHRRAM();

	if (reg[0] &0x40)
		EMU->Mirror_V();
	else
		EMU->Mirror_H();
}

void MAPINT writeReg (int, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

void MAPINT reset (RESET_TYPE) {
	for (auto& r: reg) r =0;
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	sync();
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode == STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =226;
} // namespace

MapperInfo MapperInfo_226 ={
	&mapperNum,
	_T("0380/910307"),
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