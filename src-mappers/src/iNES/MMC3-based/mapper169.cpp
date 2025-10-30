#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"
#include "..\..\Hardware\simplefdc.hpp"

#define VCD_INTERFACE 0

namespace {
uint8_t reg4800;
uint8_t reg5500;
uint8_t reg5501;
uint8_t ramLatch;
uint16_t keyboardRow;
FDC fdc;
bool pa00;
bool pa09;
bool pa13;
int pa0809;

#if VCD_INTERFACE
uint32_t vcdPosition;
uint8_t vcdBitsLeft;
uint8_t vcdLatch;
uint8_t vcdInterface;
#endif

static const uint8_t keyMap[14][8] ={
	{DIK_ESCAPE, DIK_F9,         DIK_7,      DIK_R,       DIK_A,      0x00,          0x00,         DIK_LSHIFT},
	{0x00,       DIK_NUMPADENTER,0x00,       DIK_MULTIPLY,DIK_DIVIDE, DIK_UP,        DIK_BACKSPACE,DIK_F12   },
	{0x00,       0x00,           0x00,       DIK_ADD,     DIK_NUMLOCK,DIK_LEFT,      DIK_DOWN,     DIK_RIGHT },
	{0x00,       DIK_NUMPAD7,    DIK_NUMPAD8,DIK_SUBTRACT,DIK_NUMPAD9,DIK_F11,       DIK_END,      DIK_PGDN  },
	{DIK_F8,     DIK_6,          DIK_E,      DIK_RBRACKET,DIK_L,      DIK_Z,         DIK_N,        DIK_SPACE },
	{DIK_F7,     DIK_5,          DIK_W,      DIK_LBRACKET,DIK_K,      DIK_DELETE,    DIK_B,        DIK_SLASH },
	{DIK_F6,     DIK_4,          DIK_Q,      DIK_P,       DIK_J,      DIK_BACKSLASH, DIK_V,        DIK_PERIOD},
	{DIK_F5,     DIK_3,          DIK_EQUALS, DIK_O,       DIK_H,      DIK_APOSTROPHE,DIK_C,        DIK_COMMA },
	{DIK_F4,     DIK_2,          DIK_MINUS,  DIK_I,       DIK_G,      DIK_SEMICOLON, DIK_X,        DIK_M     },
	{DIK_F3,     DIK_1,          DIK_0,      DIK_U,       DIK_F,      DIK_CAPITAL,   DIK_LWIN,     0x00      },
	{DIK_F2,     DIK_GRAVE,      DIK_9,      DIK_Y,       DIK_D,      DIK_LCONTROL,  DIK_LMENU,    DIK_SCROLL},
	{DIK_F1,     DIK_RETURN,     DIK_8,      DIK_T,       DIK_S,      0x00,          DIK_RWIN,     DIK_APPS  },
	{DIK_DECIMAL,DIK_F10,        DIK_NUMPAD4,DIK_NUMPAD6, DIK_NUMPAD5,DIK_INSERT,    DIK_HOME,     DIK_PGUP  },
	{0x00,       DIK_NUMPAD0,    DIK_NUMPAD1,DIK_NUMPAD3, DIK_NUMPAD2,DIK_SYSRQ,     DIK_TAB,      DIK_PAUSE },
};

void MAPINT writeRAMLatch (int, int, int);
int  MAPINT ppuRead1bpp (int, int);
void MAPINT ppuWrite1bpp (int, int, int);
int  MAPINT ppuReadNTSplit (int, int);
void MAPINT ppuWriteCHRSplit (int, int, int);

void sync (void) {
	if (reg5501 &0x80) { // MMC3 game mode
		MMC3::syncPRG_RAM(0x3F, 0x00);
		MMC3::syncCHR_RAM(0xFF, 0x00);
		MMC3::syncMirror();
		for (int bank =0x8; bank <=0xF; bank++) {
			EMU->SetCPUWriteHandler(bank, MMC3::write);
			EMU->SetPRG_Ptr4(bank, EMU->GetPRG_Ptr4(bank), FALSE);
		}
		for (int bank =0; bank <12; bank++) {
			EMU->SetPPUReadHandler (bank, EMU->ReadCHR);
			EMU->SetPPUReadHandlerDebug(bank, EMU->ReadCHRDebug);
			EMU->SetPPUWriteHandler (bank, EMU->WriteCHR);
		}
	} else { // Normal computer mode
		if (reg5500 &0x04) { // Map RAM
			if (reg5500 &0x40)
				EMU->SetPRG_RAM32(0x8, ramLatch >>1);
			else {
				EMU->SetPRG_RAM16(0x8, ramLatch);
				EMU->SetPRG_RAM16(0xC, 0xFF);
			}
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, reg4800 &0x20? writeRAMLatch: EMU->WritePRG);
		} else { // Map ROM
			EMU->SetPRG_ROM32(0x8, reg4800 &0x1F);
			for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, EMU->WritePRG);
		}
				
		if (reg4800 &0x80) { // Special CHR-RAM modes
			if (reg5500 &0x80) { // Automatically divide a screen page into four 8 KiB CHR-RAM banks
				for (int bank =0; bank < 8; bank++) EMU->SetPPUReadHandler(bank, EMU->ReadCHR);
				for (int bank =8; bank <12; bank++) EMU->SetPPUReadHandler(bank, ppuReadNTSplit);
				for (int bank =0; bank <12; bank++) EMU->SetPPUWriteHandler(bank, ppuWriteCHRSplit);
			} else {
				EMU->SetCHR_RAM8(0x0, reg5501 &0x10 | reg5501 <<2 &0x0C | reg5501 >>2 &0x03);
				for (int bank =0; bank <12; bank++) { // 1 bpp all-points-addressable mode
					EMU->SetPPUReadHandler (bank, ppuRead1bpp);
					EMU->SetPPUWriteHandler(bank, ppuWrite1bpp);
				}
			}
		} else // Normal CHR-RAM mode
		for (int bank =0; bank <12; bank++) {
			EMU->SetPPUReadHandler (bank, EMU->ReadCHR);
			EMU->SetPPUWriteHandler(bank, EMU->WriteCHR);
			EMU->SetCHR_RAM8(0x0, reg5501 &0x10 | reg5501 <<2 &0x0C | reg5501 >>2 &0x03);
		}
		for (int bank =0; bank <12; bank++) EMU->SetPPUReadHandlerDebug(bank, EMU->ReadCHRDebug);
		
		if (reg5500 &0x08)
			EMU->Mirror_H();
		else
			EMU->Mirror_V();
	}
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, EMU->WritePRG);
	EMU->SetPRG_RAM8(0x6, (reg5500 &0x03) ==0? 0x3C: (reg5500 &0x03 | 0x00));
}

#define BIT_CS 0x08
#define BIT_DAT 0x04
#define BIT_CLK 0x02

int MAPINT readReg (int bank, int addr) {
	int result =bank ==4? EMU->ReadAPU(bank, addr): *EMU->OpenBus;
	switch (bank <<12 |addr) {
		#if VCD_INTERFACE
		case 0x4016:
			if (~vcdInterface &BIT_CS && vcdInterface &BIT_DAT) result =result &~0x02 | (vcdLatch &0x01? 0x00: 0x02);
			break;
		case 0x4017:
			if (~vcdInterface &BIT_CS) result =result &~0x06 | (vcdInterface &BIT_CLK && vcdPosition <ROM->MiscROMSize? 0x04: 0x00) | vcdInterface &BIT_CLK;
			break;
		#endif VCD_INTERFACE
		case 0x4207:
			result =0;
			#if VCD_INTERFACE
			if (vcdInterface &BIT_CLK && vcdPosition <ROM->MiscROMSize)
				result =ROM->MiscROMData[vcdPosition++];
			else
			#endif
			for (int row =0; row <14; row++)
				for (int column =0; column <8; column++)
					if (keyboardRow &(1 <<row)) result =result >>1 | EMU->keyState[keyMap[row][column]] &0x80;
			break;
		case 0x4204: case 0x4205:
		case 0x4304: case 0x4305:
			result =fdc.readIO(addr &7);
			break;
		case 0x5002:
			result =0x02; // 00: VCD screen, 02: No VCD Screen
			break;
	}
	return result;
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (bank ==4) EMU->WriteAPU(bank, addr, val);
	switch (bank <<12 | addr) {
		#if VCD_INTERFACE
		case 0x4016:
			if (~val &BIT_CS && ~reg5501 &0x80) { // Interface selected and not in MMC3 mode
				if (~vcdInterface &BIT_CLK && val &BIT_CLK) // Rising CLK
					vcdLatch =vcdLatch >>1 | (vcdInterface &BIT_DAT? 0x80: 0x00);
				else
				if (vcdInterface &val &BIT_CLK && ~vcdInterface &BIT_DAT && val &BIT_DAT) switch(vcdLatch) { // Rising DAT
					case 0x06:
						if (!((vcdPosition -0x800) &0x1FFF)) EMU->StatusOut(L"Received %d percent from VCD", vcdPosition *100 /ROM->MiscROMSize);
						break;
					case 0x15:
						EMU->DbgOut(L"VCD reception error");
						break;
					default:
						EMU->DbgOut(L"Unknown VCD command %02X", vcdLatch);
				}
			}
			vcdInterface =val &0x0F;
			break;
		#endif
		case 0x4200: case 0x4300: // FDC Digital Control Register
			fdc.writeIO(7, val);
			break;
		case 0x4201: case 0x4301: // FDC Digital Output Register
			fdc.writeIO(2, val);
			break;
		case 0x4202: case 0x4302: case 0x5004:
			keyboardRow =keyboardRow &0xFF00 | val &0xFF;
			break;
		case 0x4203: case 0x4303: case 0x5005:
			keyboardRow =keyboardRow &0x00FF | val <<8;
			break;
		case 0x4205: case 0x4305: // FDC Data Register
			fdc.writeIO(5, val);
			break;
		case 0x4800:
			reg4800 =val;
			sync();
			break;
		#if VCD_INTERFACE
		case 0x5000: // Get VCD Player ID
			vcdLatch =0x01;
			vcdBitsLeft =8;
			break;
		case 0x5001: // VCD Player Reset
			vcdPosition =0;
			vcdBitsLeft =0;
			break;
		#endif
		case 0x5500:
			reg5500 =val;
			sync();
			break;
		case 0x5501:
			reg5501 =val;
			sync();
			break;
	}
}

void MAPINT writeRAMLatch (int bank, int addr, int val) {
	ramLatch =val;
	sync();
}

void checkMode1bpp (int bank, int addr) { 
	if (!pa13 && !!(bank &8)) { // During rising edge of PPU A13, PPU A8/A9 is latched.
		pa00 =!!(addr &0x001);
		pa09 =!!(addr &0x200);
	}
	pa13 =!!(bank &8);
}

int MAPINT ppuRead1bpp (int bank, int addr) {
	checkMode1bpp(bank, addr);
	if (!pa13)
		return EMU->ReadCHR(bank &3 | (pa09? 4: 0), addr &~8 | (pa00? 8: 0));
	else
		return EMU->ReadCHR(bank, addr);
}

void MAPINT ppuWrite1bpp (int bank, int addr, int val) {
	checkMode1bpp(bank, addr);
	EMU->WriteCHR(bank, addr, val);
}

int MAPINT ppuReadNTSplit (int bank, int addr) {
	if (!(addr &0xFF)) {
		pa0809 =addr;
		EMU->SetCHR_RAM4(0x0, reg5501 >>1 &0x06 | reg5501 <<3 &0x18);
		EMU->SetCHR_RAM4(0x4, addr >>7 &0x06    | reg5501 <<3 &0x18 | 0x01);
	}
	return EMU->ReadCHR(bank, addr);
}

void MAPINT ppuWriteCHRSplit (int bank, int addr, int val) {
	EMU->SetCHR_RAM8(0x0, reg5501 &0x10 | reg5501 <<2 &0x0C | reg5501 >>2 &0x03);
	EMU->WriteCHR(bank, addr, val);
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		reg4800 =0;
		reg5500 =0;
		reg5501 =0;
		ramLatch =0;
		keyboardRow =0;
		pa00 =false;
		pa09 =false;
		pa13 =false;
		pa0809 =0;
	}
	fdc.reset();
	MMC3::reset(resetType);
	for (int bank =0x4; bank <=0x5; bank++) {
		EMU->SetCPUReadHandler(bank, readReg);
		EMU->SetCPUWriteHandler(bank, writeReg);
	}
	for (int bank =0x6; bank <=0x7; bank++) EMU->SetCPUWriteHandler(bank, EMU->WritePRG);
	sync();
}

void MAPINT cpuCycle (void) {
	if (ROM->changedDisk35) {
		fdc.ejectDisk(0);
		if (ROM->diskData35 !=NULL) fdc.insertDisk(0, &(*ROM->diskData35)[0], ROM->diskData35->size(), true, &ROM->modifiedDisk35);
		ROM->changedDisk35 =false;
	}
	fdc.run();
	if (reg5501 &0x80) MMC3::cpuCycle();
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, reg4800);
	SAVELOAD_BYTE(stateMode, offset, data, reg5500);
	SAVELOAD_BYTE(stateMode, offset, data, reg5501);
	SAVELOAD_BYTE(stateMode, offset, data, ramLatch);
	SAVELOAD_WORD(stateMode, offset, data, keyboardRow);
	if (stateMode== STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =169;
} // namespace

MapperInfo MapperInfo_169 ={
	&mapperNum,
	_T("裕兴 Educational Computers"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
