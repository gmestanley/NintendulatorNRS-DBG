#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		reg[6];

void	MAPINT	writeLatch (int, int, int);
void	sync (void) {
	int prgAND =reg[5] <<1 &0x3E |1;
	int prgOR  =reg[1] <<1 &0x3E | reg[2] <<6;
	int chrAND =reg[4] <<3 &0xF8 |7;
	if ((reg[3] &0x0C) ==0x04 || reg[4] &0x80)
		EMU->SetPRG_ROM32(0x8, MMC3::reg[6] &(prgAND >>2) |(prgOR >>2));
	else
	if ((reg[3] &0x08) ==0x08) {
		EMU->SetPRG_ROM16(0x8, MMC3::reg[6] &(prgAND >>1) |(prgOR >>1));
		EMU->SetPRG_ROM16(0xC,         0xFF &(prgAND >>1) |(prgOR >>1));
	}
	else
		MMC3::syncPRG(prgAND, prgOR);

	MMC3::syncCHR(chrAND, 0x00);
	if (!ROM->CHRROMSize && reg[3] &1) protectCHRRAM();
	
	if ((reg[3] &0x0C) ==0x04) {
		if (MMC3::reg[6] &0x10)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	} else
	if (~reg[4] &0x40 || MMC3::mirroring &0x02) {
		if (MMC3::mirroring &1)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	} else {
		if (MMC3::mirroring &1)
			EMU->Mirror_S1();
		else
			EMU->Mirror_S0();
	}

	EMU->SetPRG_RAM8(0x6, reg[3] >>6);
	
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, reg[3] &0x0C? writeLatch: MMC3::write);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr <0x020)
		EMU->WriteAPU(bank, addr, val);
	else
	if (addr >=0xFF0 && addr <0xFF6 && ~reg[3] &0x02) {
		reg[addr &7] =val;
		sync();
	}
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	MMC3::reg[6] =val;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0;
	if (ROM->INES2_SubMapper) { // extracts
		reg[4] =0x1F | (ROM->INES2_SubMapper &1? 0x40: 0x00) | (ROM->INES2_SubMapper &2? 0x80: 0x00); //max. 256 KiB CHR
		reg[5] =0x1F; // max. 512 KiB PRG
	} else	// multicarts
		reg[5] =0x03;
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =479;
} // namespace

MapperInfo MapperInfo_479 = {
	&mapperNum,
	_T("CoolX Lite"),
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