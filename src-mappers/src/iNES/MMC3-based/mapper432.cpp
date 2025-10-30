#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

/*
Submapper 0 - 8043/GN-30C, 8086, 8090: "normal"
Submapper 1 - 8023 PCB: D5 selects solder pad
Submapper 2 - 8085 PCB: D5 selects NROM-256 mode
Submapper 3 - 8145 PCB: D5 selects 1->2 KiB MMC3 CHR mode, like mapper 197.2
*/

namespace {
FCPURead	readCart;
uint8_t		reg[2];

int	MAPINT	readPad (int, int);
void	sync (void) {
	int prgAND =reg[1] &0x02? 0x0F: 0x1F;
	int prgOR  =reg[1] <<4 &0x10 | reg[1] <<1 &0x60;
	int chrAND =reg[1] &0x04? 0x7F: 0xFF;
	int chrOR  =reg[1] <<7 &0x80 | reg[1] <<5 &0x100 | reg[1] <<4 &0x200;
	bool pad     =ROM->INES2_SubMapper ==1? !!(reg[1] &0x20): !!(reg[0] &0x01);

	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	if (ROM->INES2_SubMapper ==3 && reg[1] &0x20) {
		chrAND |=0x100;
		chrAND >>=1;
		chrOR  >>=1;
		EMU->SetCHR_ROM2(0x0, MMC3::getCHRBank(0) &chrAND | chrOR &~chrAND);
		EMU->SetCHR_ROM2(0x2, MMC3::getCHRBank(3) &chrAND | chrOR &~chrAND);
		EMU->SetCHR_ROM2(0x4, MMC3::getCHRBank(4) &chrAND | chrOR &~chrAND);
		EMU->SetCHR_ROM2(0x6, MMC3::getCHRBank(7) &chrAND | chrOR &~chrAND);
	} else
		MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncWRAM(0);
	MMC3::syncMirror();
	for (int bank =0x8; bank <0xF; bank++) EMU->SetCPUReadHandler(bank, pad? readPad: readCart);
}

int getPRGBank(int bank) {
	if (reg[1] &0x40) {
		int mask = reg[1] &(ROM->INES2_SubMapper == 2? 0x20: 0x80)? 3: 1;
		return MMC3::getPRGBank(bank & 1) &~mask | bank &mask;
	}
	else
		return MMC3::getPRGBank(bank);
}

int	MAPINT	readPad (int bank, int addr) {
	return ROM->dipValue;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (ROM->INES2_SubMapper ==3 && reg[1] &0x80)
		;
	else
		reg[addr &1] =val &0xFF;
	sync();
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, writeReg);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	readCart =EMU->GetCPUReadHandler(0x8);
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =432;
} // namespace

MapperInfo MapperInfo_432 = {
	&mapperNum,
	_T("Realtec 8023/8043/8086/8090/8286, GN-30C"),
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
