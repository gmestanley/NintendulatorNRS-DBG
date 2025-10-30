#include "..\DLL\d_iNES.h"
#include "..\Hardware\h_Latch.h"
#include "..\Hardware\Sound\s_LPC10.h"


namespace {
LPC10 lpc(&lpcABM, 640000, 1789773);
uint8_t pa0809;
uint16_t lastAddr;

void sync0 (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data &0x1F);
	EMU->SetPRG_RAM8(6, Latch::data >>6);
	EMU->SetCHR_RAM8(0, 0);
	if (Latch::data &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void sync1 (void) {
	EMU->SetPRG_ROM32(0x8, Latch::data &0x1F);
	EMU->SetPRG_RAM8(6, Latch::data >>6);
	if (Latch::data &0x20) {
		EMU->SetCHR_RAM4(0x0, pa0809);
		EMU->SetCHR_RAM4(0x4, 3);
	} else
		EMU->SetCHR_RAM8(0, 0);
	iNES_SetMirroring();
}

int MAPINT readLPC (int, int) {
	return lpc.getFinishedState()? 0x8F: lpc.getReadyState()? 0x80: 0x00;
}

void MAPINT writeLPC (int, int addr, int val) {
	if (addr ==2) lpc.writeBitsLSB(8, val);
}

BOOL MAPINT load (void) {
	Latch::load(ROM->INES2_SubMapper ==1? sync1: sync0, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	Latch::reset(RESET_HARD);
	EMU->SetCPUReadHandler (0x5, readLPC);
	EMU->SetCPUReadHandlerDebug(0x5, readLPC);
	EMU->SetCPUWriteHandler (0x5, writeLPC);
	lpc.reset();
}

void MAPINT cpuCycle (void) {
	lpc.run();
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	offset =lpc.saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, pa0809); 
	SAVELOAD_WORD(stateMode, offset, data, lastAddr); 
	if (stateMode ==STATE_LOAD) Latch::sync();
	return offset;
}

void MAPINT ppuCycle (int addr, int scanline, int cycle, int) {
	if (ROM->INES2_SubMapper ==1 && Latch::data &0x20 && ~lastAddr &0x2000 && addr &0x2000) {
		uint8_t newpa0809 =addr >>8 &3;
		if (pa0809 !=newpa0809) {
			pa0809 =newpa0809;
			Latch::sync();
		}
	}
	lastAddr =addr;
}

static int MAPINT mapperSound (int cycles) {
	return lpc.getAudio(cycles);
}

uint16_t mapperNum =177;
} // namespace

MapperInfo MapperInfo_177 = {
	&mapperNum,
	_T("Hengge Dianzi"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	mapperSound,
	NULL
};