#include	"..\DLL\d_iNES.h"

namespace VRC1 {
uint8_t		prg[3];
uint8_t		chr[2];
uint8_t		misc;
FSync		sync;

void	syncPRG (int AND, int OR) {
	EMU->SetPRG_ROM8(0x8, prg[0] &AND |OR);
	EMU->SetPRG_ROM8(0xA, prg[1] &AND |OR);
	EMU->SetPRG_ROM8(0xC, prg[2] &AND |OR);
	EMU->SetPRG_ROM8(0xE, 0xFF   &AND |OR);
}

void	syncCHR (int AND, int OR) {
	if (ROM->CHRROMSize) {
		EMU->SetCHR_ROM4(0, (chr[0] &0x0F | misc <<3 &0x10) &AND |OR);
		EMU->SetCHR_ROM4(4, (chr[1] &0x0F | misc <<2 &0x10) &AND |OR);
	} else {
		EMU->SetCHR_RAM4(0, (chr[0] &0x0F | misc <<3 &0x10) &AND |OR);
		EMU->SetCHR_RAM4(4, (chr[1] &0x0F | misc <<2 &0x10) &AND |OR);
	}
}

void	syncMirror () {
	if (ROM->INES_Flags &8) // For Vs. The Goonies conversion
		EMU->Mirror_4();
	else
	if (misc &1)
		EMU->Mirror_H();
	else	
		EMU->Mirror_V();
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg[bank >>1 &3] =val;
	sync();
}

void	MAPINT	writeMisc (int bank, int addr, int val) {
	misc =val;
	sync();
}

void	MAPINT	writeCHR (int bank, int addr, int val) {
	chr[bank &1] =val;
	sync();
}

void	MAPINT	load (FSync _sync) {
	sync =_sync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (auto& c: prg) c =0;
		for (auto& c: chr) c =0;
		misc =0;
	}
	sync();
	
	EMU->SetCPUWriteHandler(0x8, writePRG);
	EMU->SetCPUWriteHandler(0x9, writeMisc);
	for (int bank =0xA; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writePRG);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, misc);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}
} // namespace
