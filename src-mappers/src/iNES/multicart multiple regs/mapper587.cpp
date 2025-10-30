#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
uint8_t reg;
	
void sync (void) {
	EMU->SetCHR_RAM8(0, 0);
	if (reg &0x02) {
		EMU->SetPRG_ROM16(0x8, reg &~7 | Latch::data &7);
		EMU->SetPRG_ROM16(0xC, reg | 7);
	} else {
		if (reg &0x80)
			EMU->SetPRG_ROM32(0x8, reg >>1 &~3 | Latch::data &3);
		else {
			EMU->SetPRG_ROM16(0x8, reg &~7 | Latch::data &7);
			EMU->SetPRG_ROM16(0xC, reg &~7 | Latch::data &7);
		}
	}
	if (reg &0x04) {
		if (reg &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else {
		if (reg &0x01)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (reg &0x80)
		Latch::write(bank, addr, val);
	else {
		reg = val;
		sync();
	}
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL, true);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	reg = 0;
	Latch::reset(RESET_HARD);
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum = 587;
} // namespace

MapperInfo MapperInfo_587 = {
	&mapperNum,
	_T("3355"), /* (K-3057) 33-in-1 */
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
