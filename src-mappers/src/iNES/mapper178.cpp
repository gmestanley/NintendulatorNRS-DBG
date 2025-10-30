#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\Sound\s_LPC10.h"

namespace {
FCPUWrite	writeAPU;
uint8_t		reg[4];
bool		enableIRQ;
bool		buttonState;
int		count;
uint8_t		solderPad;

LPC10		lpc(&lpcGeneric, 800000, 1789773);

#define		mirrorH !!(reg[0] &1)
#define		unrom   !!(reg[0] &2)
#define		nrom128 !!(reg[0] &4)
#define		prg      ((reg[1] &7) | (reg[2] <<3))
#define		extBank    reg[3]

int	MAPINT	readSolderPad (int bank, int addr) {
	return EMU->ReadPRG(bank, addr) &~3 | ROM->dipValue &3;
}

void	sync (void) {
	if (ROM->INES_MapperNum ==551) {
		EMU->SetPRG_RAM8(0x6, 0);
		EMU->SetCHR_ROM8(0x0, extBank);
		iNES_SetMirroring();
	} else {
		EMU->SetPRG_RAM8(0x6, extBank);
		EMU->SetCHR_RAM8(0x0, 0);
		if (mirrorH)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
	EMU->SetPRG_ROM16(0x8, prg &~(!nrom128*!unrom  ));
	EMU->SetPRG_ROM16(0xC, prg |  !nrom128 |unrom*6);
	if (ROM->INES2_SubMapper ==3) for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUReadHandler(bank, solderPad &1? readSolderPad: EMU->ReadPRG);
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	if (addr &0x800) {
		reg[addr &3] =val;
		sync();
	}
}

void	MAPINT	writeSolderPad (int, int, int val) {
	solderPad =val;
	sync();
}

int	MAPINT	readInfrared (int, int) {
	EMU->SetIRQ(1);
	return 0;
}

void	MAPINT	cpuCycle () {
	lpc.run();
	if (ROM->INES2_SubMapper ==1 && enableIRQ) {
		bool newButtonState =!!GetKeyState(VK_LBUTTON);
		if (!buttonState && newButtonState) EMU->SetIRQ(0);
		buttonState =newButtonState;
	}
}

void	MAPINT	writeInfrared (int bank, int addr, int val) {
	enableIRQ =!!(val &0x80);
	if (!enableIRQ) EMU->SetIRQ(1);
}

int	MAPINT	readLPC (int, int) {
	return lpc.getReadyState()? 0x40: 0x00;
}

void	MAPINT	writeLPC (int, int, int val) {
	if ((val &0xF0) ==0x50) lpc.writeBitsLSB(4, val); else lpc.reset();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& r: reg) r =0x00;
	sync();
	writeAPU =EMU->GetCPUWriteHandler(0x4);
	EMU->SetCPUWriteHandler(0x4, writeReg);

	if (ROM->INES2_SubMapper==1) {
		EMU->SetCPUReadHandler(0x5, readInfrared);
		EMU->SetCPUWriteHandler(0x6, writeInfrared);
		enableIRQ =true;
		buttonState =false;
		count =0;
	} else
	if (ROM->INES2_SubMapper ==2) {
		lpc.reset();
		EMU->SetCPUReadHandler     (0x5, readLPC);
		EMU->SetCPUReadHandlerDebug(0x5, readLPC);
		EMU->SetCPUWriteHandler    (0x5, writeLPC);
	}	 else
	if (ROM->INES2_SubMapper ==3)
		EMU->SetCPUWriteHandler(0x6, writeSolderPad);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& r: reg) SAVELOAD_BYTE(stateMode, offset, data, r);
	if (ROM->INES2_SubMapper==1) {
		SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
		SAVELOAD_BOOL(stateMode, offset, data, buttonState);
	} else
	if (ROM->INES2_SubMapper==2)
		offset =lpc.saveLoad(stateMode, offset, data);
	else
	if (ROM->INES2_SubMapper==3)
		SAVELOAD_BYTE(stateMode, offset, data, solderPad);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

static int MAPINT mapperSound (int cycles) {
	if (ROM->INES2_SubMapper ==2)
		return lpc.getAudio(cycles);
	else
		return 0;
}

uint16_t mapperNum178 =178;
uint16_t mapperNum551 =551;
} // namespace

MapperInfo MapperInfo_178 ={
	&mapperNum178,
	_T("外星 FS305/南晶 NJ0430/PB030703-1x1"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	NULL,
	saveLoad,
	mapperSound,
	NULL
};
MapperInfo MapperInfo_551 ={
	&mapperNum551,
	_T("晶科泰 KT-xxx"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};