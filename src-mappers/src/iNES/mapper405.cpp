#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_EEPROM.h"
#include	"..\Hardware\simplefdc.hpp"
#include	"..\Hardware\Sound\s_LPC10.h"
#include	<queue>

#define DAC_FIFO 0
#define FIFO_SIZE 64
#define timerEnabled   !!(timerControl &0x80)
#define timerRepeat    !!(timerControl &0x40)
#define timerScanline  !!(timerControl &0x20)
#define timerPrescaler   (timerControl &0x0F)

#define IRQ_PRINTER  0x04
#define IRQ_FDC      0x08
#define IRQ_RTC      0x10
#define IRQ_PS2      0x20
#define IRQ_MOUSE    0x40
#define IRQ_TIMER    0x80

namespace {
#if DAC_FIFO
std::queue<uint8_t> dacFIFO;
int		dacCount;
bool		dacWait;
#endif

uint8_t		prg[8], chr;
uint8_t		pcm;
uint8_t		irqMask;
uint8_t		irqStatus;
uint8_t		timerControl;
uint8_t		timerLatch;
uint8_t		timerValue;
uint8_t		prescaler;
int		prevScanline;
FDC		fdc;
LPC10		lpc(&lpcSubor, 800000, 1789773);

int		ps2SendDelay;
int		ps2ReturnDelay;
int		keyboardScanDelay;
int		mouseScanDelay;
int		serialReturnDelay;
bool		ps2CommandInProgress;
uint8_t		ps2SendLatch;
uint8_t		ps2ReturnLatch;
uint8_t		ps2Control;
uint8_t		previousKeyState[256];
uint8_t		serialReturnLatch;
bool		mouseReporting;
std::queue<uint8_t> dataToPS2;
std::queue<uint8_t> dataFromPS2;
std::queue<uint8_t> dataFromSerial;

// Bandai Gamepad
uint8_t		reg4016;
uint8_t		reg4023;
uint8_t		reg4026;
EEPROM_24C02	*eeprom =NULL;

FCPURead	readAPU;
FCPURead	readAPUDebug;
FCPUWrite	writeAPU;

void	sync (void) {
	if (ROM->INES2_SubMapper ==3) for (int bank =0; bank <8; bank++) {
		EMU->SetPRG_ROM4(8 +bank, prg[bank]);
		if (prg[bank] &0x80) EMU->SetPRG_Ptr4(8 +bank, EMU->GetPRG_Ptr4(8+ bank), TRUE);
	} else
	if (ROM->INES2_SubMapper ==1)
		for (int bank =0; bank <8; bank++) EMU->SetPRG_ROM4(0x8 +bank, reg4026 <<1 &0x100 |prg[bank]);
	else
		for (int bank =0; bank <8; bank++) EMU->SetPRG_ROM4(0x8 +bank, reg4016 <<7 &0x100 |prg[bank]);
	EMU->SetPRG_RAM8(0x6, 0);
	for (int i =0; i<4; i++) EMU->SetCHR_RAM8(0x20 +i*8, i | chr <<2);
	for (int i =0; i<3; i++) EMU->SetCHR_RAM4(0x14 +i*4, i +5); // "Go! Go! Connie" in test mode expects CHR-RAM at $5000-$7FFF
}

#define CONTROLLER_A 0x80
#define CONTROLLER_B 0x40
#define CONTROLLER_START 0x10
#define CONTROLLER_SELECT 0x20
#define CONTROLLER_UP 0x08
#define CONTROLLER_DOWN 0x04
#define CONTROLLER_LEFT 0x02
#define CONTROLLER_RIGHT 0x01

int	MAPINT	read4 (int bank, int addr) {
	int result =EMU->ReadAPU(bank, addr);
	switch(addr) {
		case 0x017:
			if (ROM->INES2_SubMapper ==2) { // WIP: TV Dance Mat
				uint8_t temp =0;
				writeAPU(0x4, 0x016, 1);
				writeAPU(0x4, 0x016, 0);
				for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
				result =(temp &CONTROLLER_START? 0x04:0x00) |
				        (temp &CONTROLLER_SELECT? 0x08:0x00) |
				        (temp &CONTROLLER_B? 0x10:0x00) |
					(temp &CONTROLLER_RIGHT? 0x00:0x00) |
				        (temp &CONTROLLER_DOWN? 0x00:0x00) |
					(temp &CONTROLLER_LEFT? 0x00:0x00) |
					(temp &CONTROLLER_UP? 0x00:0x00)
				;
				return result ^0x00;
			} else
				return readAPU(0x4, 0x017);
		case 0x020:			
			return ps2ReturnLatch;
		case 0x022:
			return ps2Control &0x85;
		case 0x023:
			return reg4023;
		case 0x024:
			return serialReturnLatch;
		case 0x026:
			result =reg4026;
			if (eeprom && ~reg4026 &0x80)
				result =eeprom->getData()? 0x01: 0x00;
			else
			if (ROM->InputType ==INPUT_CITY_PATROLMAN)
				result &=~0x10;
			else
			if (ROM->InputType ==INPUT_UM6578_KBD_MOUSE)
				result &=0x7F;
			else
			if (ROM->InputType ==INPUT_UM6578_PS2_MOUSE)
				result |=0x09; // Go! Go! Connie-chan test (0x08) and power-off buttons (0x01)
			else
			if (ROM->InputType ==INPUT_BANDAI_GAMEPAD) {
				// Bandai Gamepad reads the controller via $4026.
				// Read standard controller, then remap bits.
				if (reg4026 ==0xFF && reg4016 ==0xF9) {
					uint8_t temp =0;
					writeAPU(0x4, 0x016, 1);
					writeAPU(0x4, 0x016, 0);
					for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
					result =(temp &CONTROLLER_START? 0x04:0x00) ^0xFF;
				} else
				if (reg4026 ==0xFF && reg4016 ==0xFE) {
					uint8_t temp =0;
					writeAPU(0x4, 0x016, 1);
					writeAPU(0x4, 0x016, 0);
					for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
					result =(temp &CONTROLLER_B? 0x04:0x00) |
						(temp &CONTROLLER_A? 0x08:0x00) |
						(temp &CONTROLLER_RIGHT? 0x10:0x00) |
						(temp &CONTROLLER_DOWN? 0x20:0x00) |
						(temp &CONTROLLER_LEFT? 0x40:0x00) |
						(temp &CONTROLLER_UP? 0x80:0x00)
					;
					result ^=0xFF;
				}
			} else
			if (ROM->INES2_SubMapper ==2) { // WIP TV Dance Mat
				uint8_t temp =0;
				writeAPU(0x4, 0x016, 1);
				writeAPU(0x4, 0x016, 0);
				for (int bit =0; bit <8; bit++) temp =temp <<1 | readAPU(0x4, 0x016) &1;
				result =(temp &CONTROLLER_B? 0x00:0x00) |
				        (temp &CONTROLLER_A? 0x40:0x00) |
				        (temp &CONTROLLER_RIGHT? 0x80:0x00) |
				        (temp &CONTROLLER_DOWN? 0x04:0x00) |
					(temp &CONTROLLER_LEFT? 0x10:0x00) |
					(temp &CONTROLLER_UP? 0x02:0x00)
				;
				return result ^0xFF;
			} else
			if (ROM->PRGROMCRC32 ==0x865BEF26) { // おジャ魔女どれみのTVで Magical Cooking
				if (reg4026 ==0x10) result =0x80; else
				if (reg4026 ==0x30) result =0x80; else
				if (reg4026 ==0x50) result =0x00; else
				EMU->DbgOut(L"reg4026 =%02X", reg4026);
			}
			break;
		case 0x027:
			result =pcm;
			break;
		case 0x032:
			result =irqMask;
			break;
		case 0x033:
			result =irqStatus;
			break;
		case 0x034:
			result =timerControl;
			break;
		case 0x035:
			result =timerLatch;
			break;
		case 0x036:
			result =timerValue;
			break;
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			result =prg[addr &7];
			break;
		case 0x200: case 0x201: case 0x202: case 0x203: case 0x204: case 0x205: case 0x206: case 0x207:
			if (ROM->INES2_SubMapper ==3) result =fdc.readIO(addr);
			break;
		case 0x302:			
			if (ROM->INES2_SubMapper ==3)
				result =lpc.getFinishedState()? 0x8F: lpc.getReadyState()? 0x80: 0x00;
			break;
	}
	return result;
}

int	MAPINT	read4Debug (int bank, int addr) {
	switch(addr) {
		case 0x020:			
			return ps2ReturnLatch;
		case 0x022:
			return ps2Control;
		case 0x023:
			return reg4023;
		case 0x027:
			return pcm;
		case 0x032:
			return irqMask;
		case 0x033:
			return readAPUDebug(bank, addr) | irqStatus;
		case 0x034:
			return timerControl;
		case 0x035:
			return timerLatch;
		case 0x036:
			return timerValue;
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			return prg[addr &7];
		default:
			return readAPUDebug(bank, addr);
	}
}

uint8_t	adpcmShift;
int	adpcmCount;

static const uint8_t mouseID[] ={
	0x4D, 0x33,                                     // Old Mouse ID: Identifies a mouse for old microsoft mode drivers
	0x08,                                           // Begin Pnp:    "(" indicates PnP IDs will follow
	0x01, 0x24,                                     // Pnp Rev:      Identifies PnP version 1.0
	0x28, 0x34, 0x2B,                               // EISA ID:      "HTK" ( A mouse company )
	0x10, 0x10, 0x10, 0x11,                         // Product ID:   "0001" ( Unique product identifier )
	0x3C,                                           // Extended:     "\"
	0x3C, 0x2D, 0x2F, 0x35, 0x33, 0x25,             // Class Name:   "\MOUSE" fits a defined Windows 95 class name
	0x3C, 0x30, 0x2E, 0x30, 0x10, 0x26, 0x10, 0x23, // Driver ID:    "\PNP0F0C" fits a defined Windows 95 microsoft mouse compatible ID
	0x19, 0x12,                                     // Check sum:    Check all characters from Begin PnP to End PnP, exclusive of the checksum characters themselves.
	0x09                                            // End PnP:      ")" indicates PnP IDs complete
};


void	MAPINT	write4 (int bank, int addr, int val) {
	writeAPU(bank, addr, val);
	switch(addr) {
		case 0x016:
			if (ROM->INES2_SubMapper ==2 && ~reg4016 &1 && val &1) { // WIP TV Dance Mat
				// D0: CLK
				// D1: DAT
				adpcmShift =adpcmShift <<1 | val >>1 &1;
				adpcmCount =(adpcmCount +1) &7;
				if (adpcmCount ==0) EMU->DbgOut(L"ADPCM: %02X", adpcmShift &0xFF);
			}
			reg4016 =val;
			sync();
			break;
		case 0x021:
			ps2SendLatch =val;
			break;
		case 0x022:
			if ((ROM->InputType ==INPUT_UM6578_KBD_MOUSE || ROM->InputType ==INPUT_UM6578_PS2_MOUSE) && val &0x01 && ~ps2Control &0x80 && val &0x80) {
				dataToPS2.push(ps2SendLatch);
				ps2CommandInProgress =true;
				while (!dataFromPS2.empty()) dataFromPS2.pop();
				dataFromPS2.push(0xFA);
				if (val &0x04) ps2SendDelay =1663;
			}
			ps2Control =val;
			break;
		case 0x023:
			reg4023 =val;
			break;
		case 0x026:
			adpcmShift =0;
			adpcmCount =0;
			if (ROM->InputType ==INPUT_UM6578_KBD_MOUSE && ~reg4026 &0x02 && val &0x02) {
				for (auto& c: mouseID) dataFromSerial.push(c);
				mouseReporting =true;
			}
			reg4026 =val;
			if (eeprom && ~reg4026 &0x80) eeprom->setPins(false, !!(val &0x02), !!(val &0x01));
			break;
		case 0x027:
			#if DAC_FIFO
			if (dacFIFO.empty()) dacWait =true;
			dacFIFO.push(val);
			if (dacFIFO.size() ==FIFO_SIZE) dacWait =false;
			#else
			pcm =val;
			#endif
			break;
		case 0x032:
			irqMask =val;
			irqStatus &=~irqMask;
			break;
		case 0x034:
			timerControl =val;
			if (!timerEnabled) irqStatus &=~IRQ_TIMER;
			break;
		case 0x035:
			timerLatch =val;
			timerValue =0;
			irqStatus &=~IRQ_TIMER;
			break;
		case 0x040: case 0x041: case 0x042: case 0x043: case 0x044: case 0x045: case 0x046: case 0x047:
			prg[addr &7] =val;
			sync();
			break;
		case 0x200: case 0x201: case 0x202: case 0x203: case 0x204: case 0x205: case 0x206: case 0x207:
			if (ROM->INES2_SubMapper ==3) fdc.writeIO(addr &7, val); // SB-2000 FDC
			break;
		case 0x300:
			chr =val;
			sync();
			break;
		case 0x301:
			if (val ==0x55) EMU->DbgOut(L"Reset to NES mode, unsupported!");
			break;
		case 0x302:
			if (ROM->INES2_SubMapper ==3) lpc.writeBitsLSB(8, val); // SB-2000 LPC
			break;
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	if (ROM->INES2_PRGRAM ==0x20) eeprom =new EEPROM_24C02(0, ROM->PRGRAMData);
	if (ROM->INES2_SubMapper ==3) EMU->SetPRGROMSize(1024*1024); // SB-2000 has 512 KiB ROM and 512 KiB DRAM
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (int bank =0; bank <8; bank++) prg[bank] =bank;
	chr =0;
	pcm =0;
	irqMask =0xFF;
	irqStatus =0x00;
	timerControl =0;
	timerLatch =0;
	timerValue =0;
	prescaler =0;
	prevScanline =0;
	reg4016 =0;
	reg4026 =0x80;
	ps2SendDelay =0;
	ps2ReturnDelay =0;
	keyboardScanDelay =0;
	mouseScanDelay =0;
	serialReturnDelay =0;
	serialReturnLatch =0;
	ps2SendLatch =0;
	ps2ReturnLatch =0;
	ps2Control =0;
	ps2CommandInProgress =false;
	mouseReporting =false;
	for (auto& c: previousKeyState) c =0;
	while (!dataToPS2.empty()) dataToPS2.pop();
	while (!dataFromPS2.empty()) dataFromPS2.pop();
	while (!dataFromSerial.empty()) dataFromSerial.pop();
	#if DAC_FIFO
	while (!dacFIFO.empty()) dacFIFO.pop();
	dacCount =0;
	dacWait =false;
	#endif
	sync();

	readAPU      =EMU->GetCPUReadHandler     (0x4);
	readAPUDebug =EMU->GetCPUReadHandlerDebug(0x4);
	writeAPU     =EMU->GetCPUWriteHandler    (0x4);

	EMU->SetCPUReadHandler     (0x4, read4);
	EMU->SetCPUReadHandlerDebug(0x4, read4Debug);
	EMU->SetCPUWriteHandler    (0x4, write4);

	if (ROM->INES2_SubMapper ==3) { // SB-2000
		fdc.reset();
		lpc.reset();
	}
}

void	MAPINT	unload (void) {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

void	clockTimer() {
	if (++prescaler >=timerPrescaler +1) {
		prescaler =0;
		if (++timerValue >=timerLatch +1) {
			timerValue =0;
			if (timerEnabled) irqStatus |=IRQ_TIMER;
			if (!timerRepeat) timerControl &=~0x80;
		}
	}
}

void	MAPINT	cpuCycle () {
	if (ROM->INES2_SubMapper ==3) { // SB-2000
		if (ROM->changedDisk35) {
			fdc.ejectDisk(1);
			fdc.writeIO(2, 0x04 | 1);
			if (ROM->diskData35 !=NULL) {
				fdc.insertDisk(1, &(*ROM->diskData35)[0], ROM->diskData35->size(), true, &ROM->modifiedDisk35);
				fdc.writeIO(2, 0x04 | 1 | 0x20);
			}
			ROM->changedDisk35 =false;
		}
		fdc.run();
		if (fdc.irqRaised())
			irqStatus |=IRQ_FDC;
		else
			irqStatus &=~IRQ_FDC;
		lpc.run();
	}
	
	if (ROM->InputType ==INPUT_UM6578_KBD_MOUSE) {
		if (!dataToPS2.empty()) switch(dataToPS2.front()) {
		case 0xFF: // Reset
			dataToPS2.pop(); // command byte
			while (!dataToPS2.empty()) dataToPS2.pop();
			dataFromPS2.push(0xAA);
			ps2CommandInProgress =false;
			break;
		case 0xF0: // Select Alternate Scan Codes
			if (dataToPS2.size() >=2) {
				dataToPS2.pop(); // command byte
				dataToPS2.pop(); // option byte
			}
			ps2CommandInProgress =false;
			break;
		case 0xF2: // Read ID
			dataToPS2.pop();
			dataFromPS2.push(0x12);
			dataFromPS2.push(0x34);
			ps2CommandInProgress =false;
			break;
		case 0xED:
		case 0xF3:
		case 0xFB:
		case 0xFC:
		case 0xFD:
			if (dataToPS2.size() >=2) {
				dataToPS2.pop(); // command byte
				dataToPS2.pop(); // option byte
			}
			ps2CommandInProgress =false;
			break;
		default:
			EMU->DbgOut(L"Unhandled keyboard command: %02X", dataFromPS2.front());
			dataToPS2.pop(); // command byte
			ps2CommandInProgress =false;
			break;
		}		
		if (ps2SendDelay && !--ps2SendDelay) irqStatus |=IRQ_PS2;
		if (!dataFromPS2.empty() && !ps2ReturnDelay) ps2ReturnDelay =1663*2;
		if (!ps2SendDelay && ps2ReturnDelay && !--ps2ReturnDelay && !dataFromPS2.empty() && ps2Control &0x80) {
			ps2ReturnLatch =dataFromPS2.front();
			dataFromPS2.pop();
			irqStatus |=IRQ_PS2;
		}		
		if (++keyboardScanDelay ==1960) {
			keyboardScanDelay =0;			
			for (int key =0; key <256; key++) {
				if ((EMU->keyState[key] &0x80) ^(previousKeyState[key] &0x80)) {
					if (~EMU->keyState[key] &0x80) dataFromPS2.push(0xF0);
					dataFromPS2.push(key &0x7F | ~EMU->keyState[key] &0x80);
				}
				previousKeyState[key] =EMU->keyState[key];
			}
		}
		
		if (!dataFromSerial.empty() && !serialReturnDelay) serialReturnDelay =1663*5;
		if (serialReturnDelay && !--serialReturnDelay && !dataFromSerial.empty()) {
			serialReturnLatch =~dataFromSerial.front();
			dataFromSerial.pop();
			irqStatus |=IRQ_MOUSE;
		}				
		if (mouseReporting && ++mouseScanDelay ==60'000) {
			mouseScanDelay =0;
			int deltaX =EMU->mouseState->lX;
			int deltaY =EMU->mouseState->lY;
			dataFromSerial.push(0x40 +!!(EMU->mouseState->rgbButtons[1] &0x80) *0x10 +!!(EMU->mouseState->rgbButtons[0] &0x80) *0x20 +(deltaY >>4 &0x0C) +(deltaX >>6 &0x03));
			dataFromSerial.push(deltaX &0x3F);
			dataFromSerial.push(deltaY &0x3F);
			dataFromSerial.push(0);
		}
	} else
	if (ROM->InputType ==INPUT_UM6578_PS2_MOUSE) {
		if (!dataToPS2.empty()) switch(dataToPS2.front()) {
		case 0xFF: // Reset
			dataToPS2.pop(); // command byte
			mouseReporting =false;
			while (!dataToPS2.empty()) dataToPS2.pop();
			dataFromPS2.push(0xAA);
			dataFromPS2.push(0x00);
			ps2CommandInProgress =false;
			break;
		case 0xF0: // Remote mode
		case 0xF5: // Disable reporting
			dataToPS2.pop(); // command byte
			mouseReporting =false;
			ps2CommandInProgress =false;
			break;
		case 0xF4: // Enable reporting
			dataToPS2.pop(); // command byte
			mouseReporting =true;
			ps2CommandInProgress =false;
			break;
		case 0xF6: // Set defaults
			dataToPS2.pop(); // command byte
			ps2CommandInProgress =false;
			break;
		default:
			dataToPS2.pop(); // command byte
			EMU->DbgOut(L"Unhandled PS/2 mouse command: %02X", dataFromPS2.front());
			ps2CommandInProgress =false;
			break;
		}
		if (ps2SendDelay && !--ps2SendDelay) irqStatus |=IRQ_PS2;
		if (!dataFromPS2.empty() && !ps2ReturnDelay) ps2ReturnDelay =1663*2;
		if (!ps2SendDelay && ps2ReturnDelay && !--ps2ReturnDelay && !dataFromPS2.empty() && ps2Control &0x80) {
			ps2ReturnLatch =dataFromPS2.front();
			dataFromPS2.pop();
			irqStatus |=IRQ_PS2;
		}		
		if (!ps2CommandInProgress && mouseReporting && ++mouseScanDelay ==60'000) {			
			mouseScanDelay =0;
			int deltaX =EMU->mouseState->lX;
			int deltaY =-EMU->mouseState->lY;
			dataFromPS2.push(0x08 +!!(EMU->mouseState->rgbButtons[1] &0x80)*0x02 +!!(EMU->mouseState->rgbButtons[0] &0x80)*0x01 +(deltaX <0? 0x10: 0) +(deltaY <0? 0x20: 0x00));
			dataFromPS2.push(deltaX);
			dataFromPS2.push(deltaY);
		}
	}
	if (!timerScanline) clockTimer();
	EMU->SetIRQ(irqStatus &~irqMask? 0: 1);
	#if DAC_FIFO
	if (!dacWait && !dacFIFO.empty() && ++dacCount >timerLatch *(1000000.0/timerLatch)/(1000000.0/timerLatch -50)) {
		dacCount =0;
		pcm =dacFIFO.front();
		dacFIFO.pop();
	}
	#endif
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (cycle >=330 && scanline !=prevScanline && timerScanline /*&& isRendering*/) {
		prevScanline =scanline;
		clockTimer();
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, irqMask);
	SAVELOAD_BYTE(stateMode, offset, data, irqStatus);
	SAVELOAD_BYTE(stateMode, offset, data, timerControl);
	SAVELOAD_BYTE(stateMode, offset, data, timerLatch);
	SAVELOAD_BYTE(stateMode, offset, data, timerValue);
	SAVELOAD_BYTE(stateMode, offset, data, prescaler);
	SAVELOAD_LONG(stateMode, offset, data, prevScanline);
	if (eeprom) offset =eeprom->saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

static int MAPINT mapperSound (int cycles) {
	if (ROM->INES2_SubMapper ==3) // SB-2000
		return lpc.getAudio(cycles)*2 + (pcm <<7);
	else
		return pcm <<7;
}

uint16_t mapperNum =405;
} // namespace

MapperInfo MapperInfo_405 = {
	&mapperNum,
	_T("UMC UM6578"),
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

