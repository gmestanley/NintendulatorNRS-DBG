#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[2];

void sync (void) {
	int prgAND =reg[1] &(ROM->INES2_SubMapper ==2? 0x01: 0x02)? 0x1F: 0x0F;
	int chrAND =reg[1] &0x02? 0xFF: 0x7F;
	int prgOR =reg[1] <<1 &0x10 | reg[1] >>1 &0x60;
	int chrOR =reg[1] <<5 &0x080 | (ROM->INES2_SubMapper ==1? (reg[1] <<2 &0x100): (reg[0] <<4 &0x100 | reg[1] <<2 &0x200));
	if (reg[0] &0x40) {
		int prg =reg[0] &0x05 | reg[0] >>2 &0x02 | prgOR >>1;
		if (reg[0] &0x02)
			EMU->SetPRG_ROM32(0x8, prg >>1);
		else {
			EMU->SetPRG_ROM16(0x8, prg);
			EMU->SetPRG_ROM16(0xC, prg);
		}
	} else {
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
	}
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
}

int MAPINT read5 (int bank, int addr) {
	return ROM->dipValue;
}

void MAPINT write5 (int bank, int addr, int val) {
	if (ROM->INES2_SubMapper ==2 || addr &0x800) reg[addr &1] =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg[0] =0x0;
	reg[1] =0x3; // Must boot up in 256 KiB mode
	
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x5, read5);
	EMU->SetCPUWriteHandler(0x5, write5);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =411;
} // namespace

MapperInfo MapperInfo_411 ={
	&mapperNum,
	_T("A88S-1"),
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
