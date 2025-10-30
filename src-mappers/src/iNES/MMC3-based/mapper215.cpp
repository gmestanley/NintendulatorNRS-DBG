#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[8];

void sync (void) {
	int prgAND =reg[0] &0x40? 0x0F: 0x1F;
	int chrAND =reg[0] &0x40? 0x7F: 0xFF;
	int prgOR =reg[1] &0x10 | reg[1] <<5 &0x60 | reg[1] <<4 &0x80;
	int chrOR =reg[1] <<2 &0x80 | reg[1] <<(ROM->INES2_SubMapper ==1? 7: 6) &0x700;	
	if (ROM->dipValueSet) {
		if (ROM->dipValue &1) {
			prgOR &=ROM->dipValue;
			chrOR &=ROM->dipValue <<3 |7;
		} else
			prgOR |=ROM->dipValue;
	}
	if (reg[0] &0x80) {
		prgAND >>=1;
		prgOR >>=1;
		int prg =reg[0] &0x0F &prgAND | prgOR &~prgAND;
		EMU->SetPRG_ROM16(0x8, reg[0] &0x20? prg &~1: prg);
		EMU->SetPRG_ROM16(0xC, reg[0] &0x20? prg | 1: prg);
	} else
		MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncWRAM(0);
	MMC3::syncCHR_ROM(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

int MAPINT readArray (int, int addr) {
	static const uint8_t arrayLUT[8][8] = {
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0 Super Hang-On
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00 }, // 1 Monkey King
		{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x00, 0x00 }, // 2 Super Hang-On/Monkey King
		{ 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x05, 0x00 }, // 3 Super Hang-On/Monkey King
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 6
		{ 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0F, 0x00 }  // 7 (default) Blood of Jurassic
	};
	return arrayLUT[reg[2]][addr &7] &0x0F | *EMU->OpenBus &0xF0;
}

void MAPINT writeExtra (int bank, int addr, int val) {
	reg[addr &7] =val;
	sync();
}

void MAPINT writeASIC (int bank, int addr, int val) {
	static const uint16_t addressLUT[8][8] ={
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 0
		{ 0xA001, 0xA000, 0x8000, 0xC000, 0x8001, 0xC001, 0xE000, 0xE001 }, // 1
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 2
		{ 0xC001, 0x8000, 0x8001, 0xA000, 0xA001, 0xE001, 0xE000, 0xC000 }, // 3
		{ 0xA001, 0x8001, 0x8000, 0xC000, 0xA000, 0xC001, 0xE000, 0xE001 }, // 4
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 5
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }, // 6
		{ 0x8000, 0x8001, 0xA000, 0xA001, 0xC000, 0xC001, 0xE000, 0xE001 }  // 7
	};
	static const uint8_t dataLUT[8][8] ={
		{ 0, 1, 2, 3, 4, 5, 6, 7 }, // 0
		{ 0, 2, 6, 1, 7, 3, 4, 5 }, // 1
		{ 0, 5, 4, 1, 7, 2, 6, 3 }, // 2
		{ 0, 6, 3, 7, 5, 2, 4, 1 }, // 3
		{ 0, 2, 5, 3, 6, 1, 7, 4 }, // 4
		{ 0, 1, 2, 3, 4, 5, 6, 7 }, // 5
		{ 0, 1, 2, 3, 4, 5, 6, 7 }, // 6
		{ 0, 1, 2, 3, 4, 5, 6, 7 }  // 7
	};
	int lutValue =addressLUT[reg[7]][bank &6 | addr &1];
	if (lutValue ==0x8000) val =val &0xC0 | dataLUT[reg[7]][val &7];
	MMC3::write(lutValue >>12, lutValue &1, val);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	if (ROM->INES_Version <2 && ROM->PRGROMSize >=2048*1024) ROM->INES2_SubMapper =1;
	MapperInfo_215.Description =ROM->INES2_SubMapper ==1? _T("Realtec 8237A"): _T("Realtec 823x");
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	reg[1] =0xFF;
	reg[2] =7; // for "Blood of Jurassic"
	reg[7] =4; // For single-cartridge "Boogerman" (should be Mapper 114.1)
	MMC3::reset(RESET_HARD);
	EMU->SetCPUReadHandler(0x5, readArray);
	EMU->SetCPUReadHandlerDebug(0x5, readArray);
	EMU->SetCPUWriteHandler(0x5, writeExtra);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeASIC);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum114 =215;
uint16_t mapperNum258 =258;
} // namespace

MapperInfo MapperInfo_215 ={
	&mapperNum114,
	_T("Realtec 823x(A)"),
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
MapperInfo MapperInfo_258 ={
	&mapperNum258,
	_T("Shanghai Paradise 158B"),
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
