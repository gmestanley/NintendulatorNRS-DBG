#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"
#include "..\..\Hardware\h_FIFO.hpp"
#include "..\..\Hardware\sound\s_MSM6585.hpp"

namespace {
uint8_t reg[4];
FIFO fifo(1024);

int serveADPCM(void) {
	return fifo.retrieve();
}

MSM6585 adpcm(1789773, serveADPCM);

void sync (void) {
	int prgAND = 0x3F;
	int chrAND = reg[2] &0xC0? 0x0FF: 0x1FF;
	int prgOR = (reg[2] &0x40? 0x0C0: 0x000) | (reg[2] &0x80? 0x100: 0x000);
	int chrOR = (reg[2] &0x40? 0x200: 0x000) | (reg[2] &0x80? 0x300: 0x000);
	EMU->SetPRG_ROM8(0x6, reg[0] | prgOR);
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

int getCHRBank (int bank) {
	return MMC3::getCHRBank(bank) | bank <<6 &0x100;
}

int MAPINT readFIFO (int bank, int addr) {
	return fifo.halfFull()? 0x00: 0x40;
}

void MAPINT writeFIFO (int bank, int addr, int val) {
	if (addr &1) {
		adpcm.setRate(val >>6);
		fifo.reset();
	} else
		fifo.add(val);
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[bank &2 | addr &1] = val;
	sync();
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, getCHRBank, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg[0] = reg[1] = reg[2] = reg[3] = 0;
	fifo.reset();
	adpcm.reset();
	MMC3::reset(resetType);	
	EMU->SetCPUReadHandler(0x5, readFIFO);
	EMU->SetCPUWriteHandler(0x5, writeFIFO);
	EMU->SetCPUWriteHandler(0x9, writeReg);
	EMU->SetCPUWriteHandler(0xB, writeReg);
}

void MAPINT cpuCycle () {
	MMC3::cpuCycle();
	adpcm.run();
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =fifo.saveLoad(stateMode, offset, data);
	offset =adpcm.saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int MAPINT mapperSound (int cycles) {
	return adpcm.getOutput();
}

uint16_t mapperNum =594;
} // namespace

MapperInfo MapperInfo_594 = {
	&mapperNum,
	_T("Rinco FSG2"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	mapperSound,
	NULL
};
