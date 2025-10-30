#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
FCPUWrite passWrite;
uint8_t index;
uint8_t lastWrite;
uint8_t ppu0000[2];
uint8_t ppu0800[2];
uint8_t ppu1000;

void sync (void) {
	MMC3::syncPRG(0x3F, 0x00);
	EMU->SetCHR_ROM2(0x0, ppu0000[0]);
	EMU->SetCHR_ROM2(0x2, ppu0800[0]);
	EMU->SetCHR_ROM4(0x4, ppu1000);
	MMC3::syncMirror();
}

void MAPINT interceptSpriteDMA (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr ==0x014) {
		ppu0000[0] =ppu0000[1];
		ppu0800[0] =ppu0800[1];
		sync();
	}
}

void MAPINT writeLatch (int bank, int addr, int val) {
	lastWrite =val;
	passWrite(bank, addr, val);
}

int MAPINT readReg (int bank, int addr) {
	if (index &0x20) {
		ppu0800[1] =(lastWrite <<1 &0x80) ^ (MMC3::getCHRBank(2) >>1);
		ppu1000 =lastWrite &0x3F;
	} else
		ppu0000[1] =lastWrite ^ (MMC3::getCHRBank(0) >>1);
	sync();
	return *EMU->OpenBus;
}

void MAPINT writeReg (int, int, int val) {
	index =val;
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, readReg, writeReg);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& r: ppu0000) r =0xFF;
	for (auto& r: ppu0800) r =0xFF;
	ppu1000 =0;
	MMC3::reset(resetType);

	passWrite =EMU->GetCPUWriteHandler(0x0);
	EMU->SetCPUWriteHandler(0x0, writeLatch);
	EMU->SetCPUWriteHandler(0x4, interceptSpriteDMA);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: ppu0000) SAVELOAD_BYTE(stateMode, offset, data, r);
	for (auto& r: ppu0800) SAVELOAD_BYTE(stateMode, offset, data, r);
	SAVELOAD_BYTE(stateMode, offset, data, ppu1000);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =292;
} // namespace

MapperInfo MapperInfo_292 = {
	&mapperNum,
	_T("BMW8544"),
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
