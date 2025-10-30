#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;
uint8_t protectionIndex;
uint8_t protectionData[4];

void sync0 (void) {
	EMU->SetPRG_ROM32(0x8, reg &1 | reg >>3 &2);
	if (reg &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	MMC3::syncCHR_ROM(0xFF, 0);
}

void sync1 (void) {
	EMU->SetPRG_ROM32(0x8, MMC3::reg[6] >>2);
	MMC3::syncMirror();
	MMC3::syncCHR_ROM(0xFF, 0);
}

BOOL MAPINT load (void) {
	MMC3::load(ROM->INES2_SubMapper ==1? sync1: sync0, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

int MAPINT readProtection (int bank, int addr) {
	return addr &0x800? protectionData[addr &3]: *EMU->OpenBus;
}

uint8_t protectionXOR (uint8_t val) {
	return 0x01*(!!(val &0x80) && !(val &0x08) || !(val &0x80) &&  !(val &0x40)) |
	       0x08*(!!(val &0x10) && !(val &0x08) || !(val &0x40) &&  !(val &0x10)) |
	       0x10*( !(val &0x40) && !(val &0x01) || !(val &0x08) && !!(val &0x01)) |
	       0x40*( !(val &0x40) && !(val &0x02) || !(val &0x08) && !!(val &0x02));
}

uint8_t protectionAND (uint8_t val) {
	return 0xA6 |
	       0x01*(!(val &0x80) && !(val &0x40) && !(val &0x20) || !!(val &0x80) &&  !(val &0x08) && !(val &0x04) ||  !(val &0x80) && !!(val &0x40) && !!(val &0x20) || !!(val &0x80) & !!(val &0x08) & !!(val &0x04)) |
	       0x08*(!(val &0x40) && !(val &0x20) && !(val &0x10) || !!(val &0x40) && !!(val &0x20) && !(val &0x10) || !!(val &0x10) &&  !(val &0x08) &&  !(val &0x04) || !!(val &0x10) & !!(val &0x08) & !!(val &0x04)) |
	       0x10*(!(val &0x40) && !(val &0x20) && !(val &0x01) || !!(val &0x40) && !!(val &0x20) && !(val &0x01) ||  !(val &0x08) &&  !(val &0x04) && !!(val &0x01) || !!(val &0x08) & !!(val &0x04) & !!(val &0x01)) |
	       0x40*(!(val &0x40) && !(val &0x20) && !(val &0x02) || !!(val &0x40) && !!(val &0x20) && !(val &0x02) ||  !(val &0x08) &&  !(val &0x04) && !!(val &0x02) || !!(val &0x08) & !!(val &0x04) & !!(val &0x02)); 
}

void MAPINT writeProtection (int bank, int addr, int val) { 
	if (addr &0x800)
		protectionData[addr &3] =(val &protectionAND(protectionIndex)) ^protectionXOR(protectionIndex);
	else
		protectionIndex =val;
}

void MAPINT writeReg (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr &0x800) {
		reg =val;
		MMC3::sync();
	}
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	protectionIndex =0;
	for (auto& r: protectionData) r =0;
	if (ROM->INES2_SubMapper !=1) {
		EMU->SetCPUWriteHandler(0x4, writeReg);
		EMU->SetCPUReadHandler(0x5, readProtection);
		EMU->SetCPUReadHandlerDebug(0x5, readProtection);
		EMU->SetCPUWriteHandler(0x5, writeProtection); 
	}
	MMC3::reset(resetType);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (ROM->INES2_SubMapper !=1) {
		SAVELOAD_BYTE(stateMode, offset, data, reg);
		SAVELOAD_BYTE(stateMode, offset, data, protectionIndex);
		for (auto& r: protectionData) SAVELOAD_BYTE(stateMode, offset, data, r);
	}
	if (stateMode ==STATE_LOAD) MMC3::sync();
	return offset;
}

uint16_t mapperNum =208;
} // namespace

MapperInfo MapperInfo_208 ={
	&mapperNum,
	_T("哥德 SL-37017"),
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
