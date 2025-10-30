#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[4];

int MAPINT readSolderPad (int, int);
void sync (void) {
	int prgAND =reg[1] &0x04? 0x0F: 0x1F;
	int chrAND =reg[1] &0x40? 0x7F: 0xFF;
	int prgOR =reg[1] <<4 &0x030 | reg[0] <<2 &0x040;
	int chrOR =reg[1] <<3 &0x180 | reg[0] <<4 &0x200;
	MMC3::syncPRG(prgAND, prgOR);
	if (reg[0] &0x08) // CNROM mode
		EMU->SetCHR_ROM8(0, reg[2] &chrAND >>3 | chrOR >>3);
	else
		MMC3::syncCHR(chrAND, chrOR);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
	for (int bank =0x8; bank<=0xF; bank++) EMU->SetCPUReadHandler(bank, reg[0] &0x40? readSolderPad: EMU->ReadPRG);
}

int getPRGBank (int bank) {
	int bankMask = reg[1] &0x80? 0: 3; // NROM mode means MMC3 register 6 applies across the entire 8000-FFFF range
	int addrMask =~reg[1] &0x80? 0: reg[1] &0x08? 1: 3; // NROM mode means that CPU A13 is used for PRG A13. NROM-256 mode means CPU A14 is used for PRG A14
	return MMC3::getPRGBank(bank &bankMask) &~addrMask | bank &addrMask;
}

int MAPINT readSolderPad (int bank, int addr) {
	return ROM->dipValue;
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (~reg[0] &0x80) {
		reg[addr &3] =val;
		sync();
	} else
	if ((addr &3) ==2) {
		reg[2] =reg[2] &~3 | val &3;
		sync();
	}
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	MMC3::reset(RESET_HARD);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =134;
} // namespace

MapperInfo MapperInfo_134 ={
	&mapperNum,
	_T("WX-KB4K/T4A54A/BS-5652"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
