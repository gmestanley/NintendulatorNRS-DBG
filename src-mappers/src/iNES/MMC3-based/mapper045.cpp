#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t index;
uint8_t reg[4];

void sync (void) {
	int prgAND =~reg[3] &0x3F;
	int chrAND =0xFF >>(~reg[2] &0xF);
	int prgOR  =reg[1] | reg[2] <<2 &0x300;
	int chrOR  =reg[0] | reg[2] <<4 &0xF00;
	
	MMC3::syncWRAM(0);
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	
	// Some circuit boards implement a simple menu selection mechanism by connecting one of four higher address lines to PRG /CE.
	if (ROM->dipValueSet && (ROM->dipValue ==1 && reg[1] &0x80 ||
		                 ROM->dipValue ==2 && reg[2] &0x40 ||
		                 ROM->dipValue ==3 && reg[1] &0x40 ||
		                 ROM->dipValue ==4 && reg[2] &0x20))
		for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);

	// Assume that CHR-RAM is unbanked when there is no CHR-ROM and CHR-RAM is specified as only 8 KiB.
	// Otherwise, assume banked CHR-ROM or banked CHR-RAM.
	if (ROM->CHRROMSize || ROM->CHRRAMSize >8192)
		MMC3::syncCHR(chrAND, chrOR &~chrAND);
	else
		EMU->SetCHR_RAM8(0x0, 0);
	
	MMC3::syncMirror();
}

int MAPINT readSolderPad (int bank, int addr) { // Solder pad on the New Years 15-in-1 cartridge
	return (~ROM->dipValue &addr)? 1: 0;
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (~reg[3] &0x40) {
		reg[index++ &3] =val;		
		sync();
	}
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	index =0;
	reg[0] =0x00;
	reg[1] =0x00;
	reg[2] =0x0F;
	reg[3] =0x00;
	MMC3::reset(resetType);
	if (ROM->dipValueSet) EMU->SetCPUReadHandler(0x5, readSolderPad);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =45;
} // namespace

MapperInfo MapperInfo_045 ={
	&mapperNum,
	_T("TC3294"),
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
