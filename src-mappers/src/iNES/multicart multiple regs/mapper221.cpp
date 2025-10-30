#include	"..\..\DLL\d_iNES.h"

namespace {
int16_t		reg[2];

void	sync (void) {
	uint8_t outerBank =reg[0] >>2 &0x38;
	uint8_t innerBank =reg[1] &0x07;
	outerBank |=ROM->INES2_SubMapper ==1? (reg[0] >>2 &0x40): (reg[0] >>3 &0x40);
	if ((outerBank <<14) >=ROM->PRGROMSize) // Selecting unpopulated banks results in open bus
		for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
	else
	if (ROM->INES2_SubMapper ==0 && reg[0] &0x0100 || ROM->INES2_SubMapper ==1 && reg[0] &0x0200) { // UNROM
		EMU->SetPRG_ROM16(0x8, outerBank | innerBank);
		EMU->SetPRG_ROM16(0xC, outerBank |         7);
	} else
	if (reg[0] &0x0002) // NROM-256
		EMU->SetPRG_ROM32(0x8, (outerBank | innerBank) >>1);
	else { // NROM-128
		EMU->SetPRG_ROM16(0x8, outerBank | innerBank);
		EMU->SetPRG_ROM16(0xC, outerBank | innerBank);
	}
	EMU->SetCHR_RAM8(0, 0);
	if (reg[1] &0x0008 && ROM->INES2_SubMapper ==0 || reg[0] &0x0400 && ROM->INES2_SubMapper ==1) protectCHRRAM();
	if (reg[0] &0x0001)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	reg[bank >>2 &1] =addr;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	sync();
	for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE mode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_WORD(mode, offset, data, c);
	if (mode ==STATE_LOAD)	sync();
	return offset;
}

uint16_t mapperNum =221;
} // namespace

MapperInfo MapperInfo_221 = {
	&mapperNum,
	_T("NTDEC N625092"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
