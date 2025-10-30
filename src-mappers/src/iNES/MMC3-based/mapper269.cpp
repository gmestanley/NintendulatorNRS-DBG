#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t index;
uint8_t reg[4];

void sync (void) {
	int prgAND =~reg[3] &0x3F;
	int chrAND =0xFF >>(~reg[2] &0xF);
	int prgOR =reg[1] | reg[3] <<2 &0x100;
	int chrOR =reg[0] | reg[2] <<4 &0xF00;
	MMC3::syncWRAM(0);
	MMC3::syncPRG(prgAND, prgOR);
	MMC3::syncCHR_ROM(chrAND, chrOR);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (~reg[3] &0x80) {
		reg[index++ &3] =val;
		sync();
	}
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	if (ROM->CHRROMSize ==0) {
		EMU->SetCHRROMSize(ROM->PRGROMSize);
		// Decrypt the CHR pattern data. Since we do not know which part of the one-bus PRG-ROM will be mapped into PPU address space, decrypt the entire ROM, and use that as our CHR-ROM.
		for (unsigned int i =0; i <ROM->PRGROMSize; i++) {
			uint8_t Val =ROM->PRGROMData[i];
			Val =((Val &1) <<6) | ((Val &2) <<3) | ((Val &4) <<0) | ((Val &8) >>3) | ((Val &16) >>3) | ((Val &32) >>2) | ((Val &64) >>1) | ((Val &128) <<0);
			ROM->CHRROMData[i] =Val;
		}
	}
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	index =0;
	reg[0] =0x00;
	reg[1] =0x00;
	reg[2] =0x0F;
	reg[3] =0x00;
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_LONG(stateMode, offset, data, index);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =269;
} // namespace


MapperInfo MapperInfo_269 = {
	&mapperNum,
	_T("Games Xplosion 121-in-1"),
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
