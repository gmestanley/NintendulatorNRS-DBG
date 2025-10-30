#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[2];
bool extMode;

void sync (void) {
	int prgAND =reg[0] &0x40? 0x0F: 0x1F;
	int chrAND =reg[0] &0x80? 0x7F: 0xFF;
	int prgOR =reg[0] <<4 &0x10 | reg[1] &0x20;
	int chrOR =reg[0] <<4 &0x80 | reg[1] <<3 &0x100;
	MMC3::syncWRAM(0);
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[addr &1] =val;
	sync();
}

void MAPINT writeASIC (int bank, int addr, int val) {
	if (~addr &1) {
		MMC3::writeReg(bank, addr, val);
		if (addr &2) extMode =!!(val &0x20);
	} else
	if (!extMode)
		MMC3::writeReg(bank, addr, val);
	else
	if (MMC3::index >=0x08 && MMC3::index <0x20) {
		int index =(MMC3::index -8) >>2;
		if (MMC3::index &1) {
			MMC3::reg[index] &=~0x0F;
			MMC3::reg[index] |=val >>1 &0x0F;
		} else {
			MMC3::reg[index] &=~0xF0;
			MMC3::reg[index] |=val <<4 &0xF0;
		}
		sync();
	} else
	if (MMC3::index >=0x25 && MMC3::index <=0x26) {
		MMC3::reg[6 | MMC3::index &1] =val >>5 &1 | val >>3 &2 | val >>1 &4 | val <<1 &8;
		sync();
	}
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	iNES_SetSRAM();
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) { 
	reg[0] =0x00;
	reg[1] =0x20;
	extMode =false;
	MMC3::reset(resetType);
	
	EMU->SetCPUWriteHandler(0x5, writeReg);
	EMU->SetCPUWriteHandler(0x8, writeASIC);
	EMU->SetCPUWriteHandler(0x9, writeASIC);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg[0]);
	SAVELOAD_BYTE(stateMode, offset, data, reg[1]);
	SAVELOAD_BOOL(stateMode, offset, data, extMode);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =219;
} // namespace

MapperInfo MapperInfo_219 = {
	&mapperNum,
	_T("卡聖 A9746"),
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
