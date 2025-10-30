#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		reg[2];

void	sync (void) {
	int prgAND =0x07 | ~reg[1] >>1 &0x38;
	int prgOR =reg[0] >>1 &0x38;
	
	EMU->SetPRG_ROM16(0x8, Latch::data &prgAND | prgOR &~prgAND);
	EMU->SetPRG_ROM16(0xC,        0x3F &prgAND | prgOR &~prgAND);
	EMU->SetCHR_RAM8(0x0, 0);
	if (Latch::data &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	int mask =reg[1] &0x80 | reg[1] >>1 &0x38;
	Latch::data =Latch::data &mask | val &~mask;
	sync();
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	Latch::reset(RESET_HARD);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =439;
} // namespace

MapperInfo MapperInfo_439 ={
	&mapperNum,
	_T("YS2309"),
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