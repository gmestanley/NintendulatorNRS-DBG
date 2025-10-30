#include "..\DLL\d_iNES.h"

namespace {
uint8_t reg[4];

void sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	int prg =(reg[0] ^reg[1]) <<1 &0x20 | (reg[2] ^reg[3]) &0x1F;
	switch(reg[1] >>2 &3) {
		case 0:
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, 0x20);
			break;
		case 1:
			EMU->SetPRG_ROM16(0x8, 0x1F);
			EMU->SetPRG_ROM16(0xC, prg);
			break;
		default:
			EMU->SetPRG_ROM16(0x8, prg | 1);
			EMU->SetPRG_ROM16(0xC, prg &~1);
			break;
	}
	EMU->SetCHR_RAM8(0x0, 0);
	if (reg[0] &0x01)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void MAPINT writeReg (int bank, int, int val) {
	reg[bank >>1 &3] =val;
	sync();
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: reg) c =0;
	sync();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =167;
} // namespace

MapperInfo MapperInfo_167 = {
	&mapperNum,
	_T("小霸王中英文电脑学习机"),
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
