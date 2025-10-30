#include "..\..\DLL\d_iNES.h"

namespace {
uint8_t reg[2];

void sync (void) {
	int prg =(reg[1] &0x40? (reg[0] >>3 &1): (reg[1] &0x01)) |
		  reg[1] &0x3E;
	int chr = reg[0] &0x03 |
		 (reg[1] &0x40? (reg[0] &0x04): (reg[1] <<2 &4)) |
		  reg[1] <<2 &0xF8;
	if (prg &0x30) prg -=0x10;
	if (chr &0xC0) chr -=0x40;
	EMU->SetPRG_ROM32(0x8, prg);
	EMU->SetCHR_ROM8(0, chr);
	if (reg[1] &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (bank ==0x4) EMU->WriteAPU(bank, addr, val);
	if (addr &0x100) {
		if (addr &0x80)
			reg[1] =val;
		else
		if (~reg[1] &0x20)
			reg[0] =val;
		sync();
	}
}

void MAPINT writeTranslate (int bank, int addr, int val) {
	if (reg[1] &0x20) {
		reg[0] =val <<3 &8 | val >>4 &7;
		sync();
	}
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	sync();
	for (int bank =0x4; bank <=0x5; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeTranslate);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =487;
} // namespace

MapperInfo MapperInfo_487 ={
	&mapperNum,
	_T("AVE NINA-08"),
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
