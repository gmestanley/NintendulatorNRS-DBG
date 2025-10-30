#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg[4];

void sync (void) {
	switch(reg[0] &0x07) {
		case 0: MMC3::syncPRG (0x1F, reg[1] <<1 &~0x1F); // MMC3, 256 KiB PRG, 256 KiB CHR
			MMC3::syncCHR_ROM(0xFF, reg[2] <<3 &~0xFF);
			break;
		case 1: MMC3::syncPRG (0x1F, reg[1] <<1 &~0x1F); // MMC3, 256 KiB PRG, 128 KiB CHR
			MMC3::syncCHR_ROM(0x7F, reg[2] <<3 &~0x7F);
			break;
		case 2: MMC3::syncPRG (0x0F, reg[1] <<1 &~0x0F); // MMC3, 128 KiB PRG, 256 KiB CHR
			MMC3::syncCHR_ROM(0xFF, reg[2] <<3 &~0xFF);
			break;
		case 3: MMC3::syncPRG (0x0F, reg[1] <<1 &~0x0F); // MMC3, 128 KiB PRG, 128 KiB CHR
			MMC3::syncCHR_ROM(0x7F, reg[2] <<3 &~0x7F);
			break;
		case 4: EMU->SetPRG_ROM16(0x8, reg[1]); // NROM-128
			EMU->SetPRG_ROM16(0xC, reg[1]);
			EMU->SetCHR_ROM8(0, reg[2]);
			break;
		case 5: EMU->SetPRG_ROM32(0x8, reg[1] >>1); // NROM-256
			EMU->SetCHR_ROM8(0, reg[2]);
			break;
		case 6: EMU->SetPRG_ROM32(0x8, reg[1] >>1); // CNROM, 16 KiB CHR
			EMU->SetCHR_ROM8(0, reg[2] &~0x01 | reg[3] &0x01);
			break;
		case 7: EMU->SetPRG_ROM32(0x8, reg[1] >>1); // CNROM, 32 KiB CHR
			EMU->SetCHR_ROM8(0, reg[2] &~0x03 | reg[3] &0x03);
			break;
	}
	if (reg[0] &0x04) {
		if (reg[3] &0x04)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	} else
		MMC3::syncMirror();
	MMC3::syncWRAM(0);
}

int MAPINT readSolderPad (int bank, int addr) {
	return ROM->dipValue &0x03 | *EMU->OpenBus &~0x03;
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (~reg[0] &0x80) {
		reg[addr &3] =val;
		sync();
	}
}

void MAPINT writeLatch (int bank, int addr, int val) {
	if (reg[0] &0x04) {
		reg[3] =val;
		sync();
	} else
		MMC3::write(bank, addr, val);
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);

	EMU->SetCPUReadHandler(0x5, readSolderPad);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =260;
} // namespace

MapperInfo MapperInfo_260 ={
	&mapperNum,
	_T("HP10xx-HP20xx"),
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
