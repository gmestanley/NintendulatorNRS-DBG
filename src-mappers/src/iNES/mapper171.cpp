#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC3.h"
#include	"..\Hardware\simplefdc.hpp"
#include	"..\Hardware\Sound\s_LPC10.h"

namespace {
uint8_t		reg[256];
int		irqCount;
uint8_t		prev09;

FDC		fdc;
LPC10		lpc(&lpcBBK, 800000, 1789773);

uint8_t              splitIndex;
uint8_t              scanlineCount;
std::vector<uint8_t> splitScanlines, tempSplitScanlines;
std::vector<uint8_t> splitBanks, tempSplitBanks;

#define splitMode !!(reg[1] &0x40)
#define enableIRQ !!(reg[1] &0x04)

FCPURead	readPPU;
FCPUWrite	writePPU;

int	MAPINT	readReg (int, int);
void	sync (void) {
	EMU->SetPRG_RAM16(0x4, 0x1E);
	if (reg[4] &0x80) {
		if (reg[0x09] &0x40 && ROM->PRGRAMSize <8*1024*1024)
			for (int bank =0x8; bank <=0xB; bank++) EMU->SetPRG_OB4(bank);
		else {
			EMU->SetPRG_RAM8(0x8, reg[0x14] | reg[0x0C] <<8 &0x100 | reg[0x09] <<3 &0x200);
			EMU->SetPRG_RAM8(0xA, reg[0x1C] | reg[0x0C] <<8 &0x100 | reg[0x09] <<3 &0x200);
		}
	} else {
		EMU->SetPRG_ROM8(0x8, reg[0x14]);
		EMU->SetPRG_ROM8(0xA, reg[0x1C]);
	}
	if (reg[0x01] &0x08 || reg[0x11] &0x80) {
		EMU->SetPRG_RAM8(0xC, reg[0x24]);
		EMU->SetPRG_RAM8(0xE, reg[0x2C]);
		EMU->SetCPUReadHandler     (0xF, reg[0x01] &0x08 || ~reg[0x09] &0x02? EMU->ReadPRG: readReg);
		EMU->SetCPUReadHandlerDebug(0xF, EMU->ReadPRG);
	} else {
		EMU->SetPRG_ROM8(0xC, 0x1FE);
		EMU->SetPRG_ROM8(0xE, 0x1FF);
		EMU->SetCPUReadHandler     (0xF, readReg);
		EMU->SetCPUReadHandlerDebug(0xF, EMU->ReadPRG);
	}
	
	if (!splitMode)
	for (int bank =0x0; bank <=0x7; bank++) EMU->SetCHR_RAM1(bank, reg[0x23 +bank*8]);

	switch(reg[1] &3) {
		case 0: EMU->Mirror_V(); break;
		case 1: EMU->Mirror_H(); break;
		case 2: EMU->Mirror_S0(); break;
		case 3: EMU->Mirror_S1(); break;
	}
}

int	MAPINT	interceptPPURead (int bank, int addr) {
	int result =readPPU(bank, addr);
	//if (addr ==2) EMU->WritePRG(0x6, 0, EMU->ReadPRG(0x6, 0) |0x80);
	return result;
}

void	MAPINT	interceptPPUWrite (int bank, int addr, int val) {
	if (addr ==0 || addr ==1 || addr ==5) {
		if (reg[0x01] &0x08 || reg[0x11] &0x80) EMU->WritePRG(bank &1 |6, addr, val);
	}
	writePPU(bank, addr, val);
}

int	MAPINT	read4 (int bank, int addr) {
	return addr &0xC00? EMU->ReadPRG(bank, addr): EMU->ReadAPU(bank, addr);
}

int	MAPINT	read4Debug (int bank, int addr) {
	return addr &0xC00? EMU->ReadPRG(bank, addr): EMU->ReadAPUDebug(bank, addr);
}

void	MAPINT	write4 (int bank, int addr, int val) {
	if (addr &0xC00)
		EMU->WritePRG(bank, addr, val);
	else
		EMU->WriteAPU(bank, addr, val);
}

int	MAPINT	readReg (int bank, int addr) {
	int result =EMU->ReadPRG(bank, addr);
	if ((addr &~7) ==0xF80)
		result =fdc.receiveFromDMA();
	else
	if ((addr &~7) ==0xF88) {
		fdc.setTerminalCount();		
		result =fdc.receiveFromDMA();
	} else
	if ((addr &~7) ==0xF90)
		result =fdc.haveDMARequest()? 0x40: 0x00;
	else
	if ((addr &~7) ==0xF98)
		result =fdc.irqRaised()? 0x40: 0x00;
	else
	if (addr ==0xFB0) {
		EMU->SetIRQ(1);
		return 6; // For scanline IRQ handler
	} else
	if (addr >=0xF98 && addr <=0xFB8)
		result =fdc.readIO(addr >>3 &7);
	else
	if (addr ==0xF18)
		result =lpc.getFinishedState()? 0x8F: lpc.getReadyState()? 0x80: 0x00;
	return result;
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (addr <0xF00)
		EMU->WritePRG(bank, addr, val);
	else
	if (addr <0xF90) {
		if ((addr &0xFF) ==0x10 && ~reg[0x10] &1 && val &1) lpc.reset();
		reg[addr &0xFF] =val;
		switch (addr &0xFF) {
			case 0x00: // Keyboard LED
				break;
			case 0x01: // Video control port
				if (!enableIRQ) EMU->SetIRQ(1);
				break;
			case 0x02: // Scanline counter lower nibble
				scanlineCount =scanlineCount &0xF0 |val &0x0F;
				break;
			case 0x03: case 0x0B: case 0x13: case 0x1B: // 2 KiB CHR-RAM bank
				reg[0x23 +(addr <<1 &0x30)] =val <<1 |0;
				reg[0x2B +(addr	<<1 &0x30)] =val <<1 |1;
				break;
			case 0x04: // 16 KiB PRG bank 8000-BFFF
				//if (val ==0xE0) val &=~0x20;
				reg[0x14] =val <<1 |0;
				reg[0x1C] =val <<1 |1;
				break;
			case 0x05: // Value 7 set on game start
				//EMU->DbgOut(L"FF05: %02X", val);
				break;
			case 0x06: // Scanline counter upper nible
				scanlineCount =scanlineCount &0x0F |val <<4;
				break;
			case 0x09: // D6 is highest RAM address bit
				   // D5 is set by Floppy 1 BIOS.
				   // D2 set on game start
				   // D1 cleared on game start
				//EMU->DbgOut(L"FF09: %02X", val);
				break;
			case 0x0A: // Add scanline to CHR bank switching queue
				tempSplitScanlines.push_back(val);
				break;
			case 0x0C: // 16 KiB PRG bank 8000-BFFF MSB?, mirrored at 5FE5
				break;
			case 0x10: // Reset LPC
				break;
			case 0x11: // decreased at start of BIOS call, increased and written here at end.
				//EMU->DbgOut(L"FF11: %02X", val);
				break;
			case 0x12: // Reset the CHR bank switching queue
				splitScanlines.clear();
				splitBanks.clear();
				splitIndex =0;
				break;
			case 0x14: case 0x1C: // 8 KiB PRG bank 8000-9FFF, A000-BFFF
				//if (val ==0xC0 || val ==0xC1) reg[addr &0xFF] &=0x3F;
				reg[0x04] =0x80; // Enable RAM in 8000-BFFF. Needed by Crossword.
				break;
			case 0x18: // LPC data
				lpc.writeBitsLSB(8, val);
				break;
			case 0x19: // Written-to at the start of a BIOS function with number
				reg[0x11] =0;
				break;
			case 0x1A: // Add bank number to CHR bank switching queue
				tempSplitBanks.push_back(val);
				break;
			case 0x22: // Start screen plan
				splitScanlines =tempSplitScanlines;
				splitBanks =tempSplitBanks;
				tempSplitScanlines.clear();
				tempSplitBanks.clear();
				break;
			case 0x2A: // Related to the CHR bank switching queue. Only use by SC-98.
				splitIndex =0;
				break;
			case 0x23: case 0x2B: case 0x33: case 0x3B: case 0x43: case 0x4B: case 0x53: case 0x5B: // 1 KiB CHR-RAM bank
				break;
			case 0x24: case 0x2C: // Bank CD/EF
				break;
			case 0x40: case 0x48: case 0x50: // Parallel port data
				break;
			case 0x80:
				fdc.sendToDMA(val);
				break;
			case 0x88:
				fdc.setTerminalCount();		
				fdc.sendToDMA(val);
				break;
			default:
				//EMU->DbgOut(L"FF%02X=%02X", addr &0xFF, val);
				break;
		}
		sync();
	} else
	if (addr ==0xFB0) { // IRQ Mask
		//EMU->DbgOut(L"FFB0=%d", val);
		reg[0xB0] =val;
		EMU->SetIRQ(1);
	} else
	if (addr ==0xF98) { // IRQ ???
		//EMU->DbgOut(L"FF98=%d", val);
		reg[0x09] =prev09;
		sync();
		EMU->SetIRQ(1);
	} else
	if (addr ==0xFA0) {
		fdc.setResetPin(!!(val &0x40));
	} else
	if (addr <=0xFB8)
		fdc.writeIO(addr >>3 &7, val);
	else
		;
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	fdc.reset();
	fdc.setDriveMask(0b0001); // For SC-98, only the first drive may respond, otherwise BBGDOS will destroy some structures.
	lpc.reset();
	sync();
	
	// Set up PPU register cache
	readPPU  =EMU->GetCPUReadHandler(0x2);
	writePPU =EMU->GetCPUWriteHandler(0x2);
	for (int bank =0x2; bank <=0x3; bank++) {
		EMU->SetCPUReadHandler(bank, interceptPPURead);
		EMU->SetCPUWriteHandler(bank, interceptPPUWrite);
	}
	
	// Set up RAM at 4020-4FFF
	EMU->SetCPUReadHandler     (0x4, read4);
	EMU->SetCPUReadHandlerDebug(0x4, read4Debug);
	EMU->SetCPUWriteHandler    (0x4, write4);

	// Set up BBK hardware registers
	EMU->SetCPUReadHandler     (0xF, readReg);
	EMU->SetCPUReadHandlerDebug(0xF, EMU->ReadPRG);
	EMU->SetCPUWriteHandler    (0xF, writeReg);
}

void	MAPINT	cpuCycle (void) {
	if (ROM->changedDisk35) {
		fdc.ejectDisk(0);
		if (ROM->diskData35 !=NULL) fdc.insertDisk(0, &(*ROM->diskData35)[0], ROM->diskData35->size(), false, &ROM->modifiedDisk35);
		ROM->changedDisk35 =false;
	}
	fdc.run();
	lpc.run();
}

bool	raiseIRQ =false;

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (cycle ==256) {
		if (splitMode) {
			if (scanline ==-1) {
				splitIndex =0;
				scanlineCount =0;
			} else
			if (scanline <241)
				scanlineCount++;
			
			if (!scanlineCount && splitIndex <splitBanks.size() && splitIndex <splitScanlines.size()) {
					EMU->SetCHR_RAM2(0, splitBanks[splitIndex] &0x0F);
					EMU->SetCHR_RAM2(2, splitBanks[splitIndex] >>4);
					scanlineCount =splitScanlines[splitIndex];
					splitIndex++;
			}
			if (scanline ==239 && enableIRQ) {
				EMU->SetIRQ(0);
				prev09 =reg[0x09];
				reg[0x09] |=0x02; // Ensure that SC-98's IRQ handler can read registers
				sync();
			}
		} else
		if (isRendering && !++scanlineCount && enableIRQ) {
			EMU->SetIRQ(0);
			prev09 =reg[0x09];
			reg[0x09] |=0x02; // Ensure that SC-98's IRQ handler can read registers
			sync();
		}
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	offset =lpc.saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

static int MAPINT mapperSound (int cycles) {
	return lpc.getAudio(cycles);
}

uint16_t mapperNum =171;
} // namespace

MapperInfo MapperInfo_171 ={
	&mapperNum,
	_T("BBK"),
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

/*
FF08	D5
FF80	DMA Acknowledge
FF88	DMA Terminal Count
FF90 R	DMA Request (D6)
FF90 W	FDC Digital Output Register (D2)
FF98	FDC IRQ (D6)
FFA0 R	FDC Master Status Register
FFA0 W	FDC Reset
FFA8	FDC Data
FFB8 R	FDC Disk Change (D7)
FFB8 W	FDC Data Rate

A78D	Read floppy
A794	Read HD
F381
5A6F

DD9B

--check disk 029
9A90
983C
97E6	interrupted by NMI that enables rendering


D62A	Disable NMI, 3rd after black

[599F]=837C
[5942]=82DE

*/
