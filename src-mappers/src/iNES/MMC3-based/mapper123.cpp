#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

void sync (void) {
	if (reg &0x40) {
		int prg =reg &0x05 | reg >>2 &0x0A;
		EMU->SetPRG_ROM16(0x8, reg &0x02? prg &~1: prg);
		EMU->SetPRG_ROM16(0xC, reg &0x02? prg | 1: prg);
	} else
		MMC3::syncPRG(0x3F, 0x00);
	MMC3::syncCHR_ROM(0xFF, 0x00);
	MMC3::syncMirror();
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (addr &0x800) {
		reg =val;
		sync();
	}
}

void MAPINT writeASIC (int bank, int addr, int val) {
	static const uint8_t lut[8] ={ 0, 3, 1, 5, 6, 7, 2, 4 };
	if (~addr &1) val =val &0xC0 | lut[val &0x7];
	MMC3::writeReg(bank, addr, val);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) reg =0;
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	for (int bank =0x8; bank<=0x9; bank++) EMU->SetCPUWriteHandler(bank, writeASIC);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =123;
} // namespace

MapperInfo MapperInfo_123 ={
	&mapperNum,
	_T("卡聖 H2288"),
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
