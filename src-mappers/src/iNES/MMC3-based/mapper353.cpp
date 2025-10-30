#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"
#include "..\..\Hardware\Sound\s_FDS.h"

namespace {
uint8_t reg;

void sync (void) {
	MMC3::syncWRAM(0);
	if (reg ==2)
		MMC3::syncPRG(0x0F, MMC3::reg[0] >>3 &0x10 | reg <<5);
	else
		MMC3::syncPRG(0x1F, reg <<5); 
	if (reg ==3 && ~MMC3::reg[0] &0x80) {
		EMU->SetPRG_ROM8(0xC, MMC3::reg[6] |0x70);
		EMU->SetPRG_ROM8(0xE, MMC3::reg[7] |0x70);
	}
	
	if (reg ==2 && MMC3::reg[0] &0x80)
		EMU->SetCHR_RAM8(0, 0);
	else
		MMC3::syncCHR_ROM(0x7F, reg <<7);
	
	if (reg ==0)
		MMC3::syncMirrorA17();
	else
		MMC3::syncMirror();
}

int MAPINT read4 (int bank, int addr) {
	return addr <0x20? EMU->ReadAPU(bank, addr): FDSsound::Read(bank <<12 |addr);
}

void MAPINT write4 (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	if (addr !=0x023) FDSsound::Write(bank <<12 |addr, val);
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (addr &0x80) {
		reg =bank >>1 &3;
		sync();
	} else
		MMC3::write(bank, addr, val);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =0;
	MMC3::reset(RESET_HARD);
	FDSsound::Reset(resetType);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	EMU->SetCPUReadHandler(0x4, read4);
	EMU->SetCPUWriteHandler(0x4, write4);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =FDSsound::SaveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

static int MAPINT mapperSnd (int cycles) {
	return *EMU->BootlegExpansionAudio? FDSsound::Get(cycles): 0;
}

uint16_t mapperNum =353;
} // namespace

MapperInfo MapperInfo_353 ={
	&mapperNum,
	_T("81-03-05-C"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	mapperSnd,
	NULL
};
