#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_Latch.h"

namespace {
uint8_t		scratch[4];
int	MAPINT	readPad (int, int);

void	sync (void) {
	if (Latch::addr &0x80) {
		EMU->SetPRG_ROM16(0x8, Latch::addr);
		EMU->SetPRG_ROM16(0xC, Latch::addr);
	} else
		EMU->SetPRG_ROM32(0x8, Latch::addr >>1);
	
	iNES_SetCHR_Auto8(0x0, Latch::data);
	
	if (Latch::data &0x80)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();

	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, Latch::addr &0x40? readPad: EMU->ReadPRG);
}

int	MAPINT	readScratch (int, int addr) {
	return addr &0x800? scratch[addr &3]: *EMU->OpenBus;
}

void	MAPINT	writeScratch (int, int addr, int val) {
	if (addr &0x800) scratch[addr &3] =val;
}

int	MAPINT	readPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr &~0xF | ROM->dipValue);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	if (addr &0x100) { // Lock everything except CHR A13..14
		bank =Latch::addr >>12;
		addr =Latch::addr &0xFFF;
		val =val &3 | Latch::data &~3;
	}
	Latch::write(bank, addr, val);
}

BOOL	MAPINT	load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: scratch) c =0;
	Latch::reset(RESET_HARD);
	EMU->SetCPUReadHandler(0x5, readScratch);
	EMU->SetCPUWriteHandler(0x5, writeScratch);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_AD(stateMode, offset, data);
	for (auto& c: scratch) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =519;
} // namespace

MapperInfo MapperInfo_519 ={
	&mapperNum,
	_T("840348C/43-163/EH8813A"),
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
