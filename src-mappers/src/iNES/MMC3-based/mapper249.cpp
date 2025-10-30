#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t reg;

static const uint8_t prgLUT[4][4] = {
	{ 3, 4, 2, 1 },
	{ 4, 3, 1, 2 },
	{ 1, 2, 3, 4 },
	{ 2, 1, 4, 3 }
};
	
static const uint8_t chrLUT[8][6] = {
	{ 5, 2, 6, 7, 4, 3 },
	{ 4, 5, 3, 2, 7, 6 },
	{ 2, 3, 4, 5, 6, 7 },
	{ 6, 4, 2, 3, 7, 5 },
	{ 5, 3, 7, 6, 2, 4 },
	{ 4, 2, 5, 6, 7, 3 },
	{ 3, 6, 4, 5, 2, 7 },
	{ 2, 5, 6, 7, 3, 4 }
};

int scrambleBankOrder (int val, const uint8_t* sourceBits, const uint8_t* targetBits, int regLength) {
	int result =0;
	for (int bit =0; bit <8; bit++) if (val &(1 <<bit)) {
		int index;
		for (index =0; index <regLength; index++) if (sourceBits[index] ==bit) break;
		result |= 1<<(index ==regLength? bit: targetBits[index]);
	}
	return result;
}

void sync (void) {
	MMC3::syncWRAM(0);	
	for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(bank *2 +0x8, scrambleBankOrder(MMC3::getPRGBank(bank), prgLUT[reg &3], prgLUT[0], 4));
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank,         scrambleBankOrder(MMC3::getCHRBank(bank), chrLUT[reg &7], chrLUT[0], 6));
	MMC3::syncMirror();
}

void MAPINT writeReg (int, int, int val) {
	reg =val;
	sync();
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg =2;
	MMC3::reset(resetType);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =249;
} // namespace

MapperInfo MapperInfo_249 ={
	&mapperNum,
	_T("43-319"),
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
