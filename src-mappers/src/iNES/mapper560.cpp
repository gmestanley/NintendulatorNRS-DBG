#include "..\DLL\d_iNES.h"

namespace {
FSync sync;
uint8_t prg;
uint8_t tileBank;
uint8_t tileRAM[1024];

int MAPINT readCHR_HongDa_0 (int bank, int addr) {
	return ROM->CHRROMData[addr <<13 &0x10000 | bank <<9 &0x00E00 | addr >>1 &0x001F8 | addr &0x00007];
}

int MAPINT readCHR_HongDa_1 (int bank, int addr) {
	return ROM->CHRROMData[tileBank <<11 &0x1F800 | bank <<9 &0x00600 | addr >>1 &0x001F8 | addr &7];
}

int MAPINT readCHR_Subor_1 (int bank, int addr) {
	if (tileBank &0xC0)
		return ~addr &8 && tileBank &0x80 || addr &8 && tileBank &0x40? ROM->CHRROMData[tileBank <<12 &0x1F000 | bank <<10 &0x00C00 | addr &0x003F0 | tileBank >>2 &0x00008 | addr &0x00007]: 0;
	else
		return EMU->ReadCHR(bank, addr);
}

int MAPINT readNT (int bank, int addr) {
	if (addr <0x03C0) tileBank =tileRAM[addr];
	return EMU->ReadCHR(bank, addr);
}

void MAPINT writeTileRAM (int bank, int addr, int val) {
	tileRAM[addr] =val;
	EMU->WriteCHR(bank, addr, val);
}

void MAPINT writePRG (int, int, int) {
	prg =prg ^ 1;
	sync();
}

void sync_HongDa (void) {
	EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetPRG_ROM32(0x8, prg);
	EMU->SetCHR_ROM4(0x0, 0);
	EMU->SetCHR_ROM4(0x4, 0);
	for (int bank = 0; bank < 8; bank++) {
		EMU->SetPPUReadHandler(bank, prg? readCHR_HongDa_1: readCHR_HongDa_0);
		EMU->SetPPUReadHandlerDebug(bank, prg? readCHR_HongDa_1: readCHR_HongDa_0);
	}
	iNES_SetMirroring();
}

void sync_Subor (void) {
	EMU->SetPRG_RAM8(0x6, 0x00);
	EMU->SetPRG_ROM32(0x8, prg);
	EMU->SetCHR_ROM8(0x0, 0);
	for (int bank = 0; bank < 8; bank++) EMU->SetPPUReadHandler(bank, prg? readCHR_Subor_1: EMU->ReadCHR);
	iNES_SetMirroring();
}

BOOL MAPINT load (void) {
	sync =ROM->INES2_SubMapper ==1? sync_Subor: sync_HongDa;
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	prg =true;
	for (auto& c: tileRAM) c =0x00;
	sync();
	for (int bank = 0x8; bank < 0xF; bank++) {
		EMU->SetPPUReadHandler(bank, readNT);
		EMU->SetPPUReadHandlerDebug(bank, EMU->ReadCHR);
		EMU->SetCPUWriteHandler(bank, writePRG);
	}
	EMU->SetPPUWriteHandler(0x9, writeTileRAM);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, tileBank);
	for (auto& c: tileRAM) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =560;
} // namespace

MapperInfo MapperInfo_560 ={
	&mapperNum,
	_T("C/E BASIC"),
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