#include "..\DLL\d_iNES.h"
#include "..\Hardware\h_Latch.h"
#include "..\Hardware\Sound\s_LPC10.h"

namespace {
LPC10 lpc(&lpcGeneric, 768000, 1789773);
uint8_t pa0809;
uint16_t lastAddr;
	
void sync (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data);
	EMU->SetCHR_RAM8(0x0, 0);
	EMU->SetPRG_RAM8(0x6, 0);
	iNES_SetMirroring();
}

#define mode1bpp !!(Latch::data &0x80)
#define chrSplit !!(Latch::data &0x40)

FPPURead readPPU;
FPPURead readPPUDebug;
FPPUWrite writePPU;
bool pa00;
bool pa09;
bool pa13;

void checkMode1bpp (int bank, int addr) {
	// During rising edge of PPU A13, PPU A9 is latched.
	bool pa13new =!!(bank &8);
	if (!pa13 && pa13new) {
		pa00 =!!(addr &0x001);
		pa09 =!!(addr &0x200);
	}
	pa13 =pa13new;
}

int MAPINT interceptPPURead1bpp (int bank, int addr) {
	checkMode1bpp(bank, addr);
	if (chrSplit && !pa13)
		return readPPU(bank &3 | (pa09? 4: 0), addr);
	else
	if (mode1bpp && !pa13)
		return readPPU(bank &3 | (pa09? 4: 0), addr &~8 | (pa00? 8: 0));
	else
		return readPPU(bank, addr);
}

void MAPINT interceptPPUWrite1bpp (int bank, int addr, int val) {
	checkMode1bpp(bank, addr);
	writePPU(bank, addr, val);
}

int MAPINT readLPC (int, int) {
	return lpc.getReadyState()? 0x40: 0x00;
}

void MAPINT writeLPC (int, int, int val) {
	if ((val &0xF0) ==0x50) lpc.writeBitsLSB(4, val); else lpc.reset();
}

BOOL MAPINT load (void) { 
	iNES_SetSRAM(); 
	Latch::load(sync, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	EMU->SetCPUReadHandler (0x5, readLPC);
	EMU->SetCPUReadHandlerDebug(0x5, readLPC);
	EMU->SetCPUWriteHandler (0x5, writeLPC);
	if (ROM->INES2_SubMapper ==1) {
		readPPU =EMU->GetPPUReadHandler(0x0);
		readPPUDebug =EMU->GetPPUReadHandler(0x0);
		writePPU =EMU->GetPPUWriteHandler(0x0);
		for (int bank =0; bank <12; bank++) {
			EMU->SetPPUReadHandler (bank, interceptPPURead1bpp);
			EMU->SetPPUReadHandlerDebug(bank, readPPUDebug);
			EMU->SetPPUWriteHandler (bank, interceptPPUWrite1bpp);
		}
	}
	lpc.reset();
}

void MAPINT cpuCycle (void) {
	lpc.run();
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	offset =lpc.saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

static int MAPINT mapperSound (int cycles) {
	return lpc.getAudio(cycles);
}

uint16_t mapperNum =241;
} // namespace

MapperInfo MapperInfo_241 ={
	&mapperNum,
	_T("BNROM with WRAM"),
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
