#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
uint8_t reg[4];

void sync (void) {
	int prg =reg[0] <<7 &0x80 | reg[1] <<1 &0x7E | reg[1] >>6 &0x01;
	if (reg[0] &0x80) {
		if (reg[1] &0x80)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
	} else {
		EMU->SetPRG_ROM16(0x8, prg &~7 | Latch::data &7);
		EMU->SetPRG_ROM16(0xC, prg | 7);
	}
	iNES_SetCHR_Auto8(0x0, reg[2] <<2 | reg[0] >>1 &3);
	if (reg[0] &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[addr &(ROM->CHRROMSize? 3: 1)] =val;
	sync();
}

BOOL MAPINT load () {
	Latch::load(sync, Latch::busConflictAND);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg[0] =0x80;
	reg[1] =0x43;
	reg[2] =0x00;
	reg[3] =0x00;
	Latch::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =314;
} // namespace

MapperInfo MapperInfo_314 ={
	&mapperNum,
	_T("64-in-1 No Repeat"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};