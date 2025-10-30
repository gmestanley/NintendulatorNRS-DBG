#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FlashROM.h"
#include	"..\Hardware\Sound\s_MP3.h"
#include	<queue>

namespace GTROM {
uint8_t		reg;
uint8_t		serialData;
int		serialCount;
FlashROM	*flash =NULL;
TCHAR		nameStem[1024];
std::queue<uint8_t> dspCommands;

void	sync (void) {
	EMU->SetPRG_ROM32(0x8, reg &0xF);
	EMU->SetCHR_RAM8(0x0, reg >>4 &1   );
	EMU->SetCHR_RAM8(0x8, reg >>5 &1 |2);
}

int	MAPINT	writeOpenBus (int bank, int addr) {
	reg =*EMU->OpenBus;
	sync();
	return reg;
}

void	MAPINT	writeReg (int bank, int addr, int val) { // C00B in Goofy Foot
	if (serialCount !=-1) {
		serialData =serialData >>1 | val &0x80;
		if (++serialCount ==8) {
			dspCommands.push(serialData);
			serialCount =-1;
		}
	} else
	if (reg &0x80 && ~val &0x80)
		serialCount++;
	reg =val;
	sync();
}

int	MAPINT	readFlash (int bank, int addr) {
	return flash->read(bank, addr);
}

void	MAPINT	writeFlash (int bank, int addr, int val) {
	flash->write(bank, bank <<12 &0x7000 | addr, val);
}

BOOL	MAPINT	load (void) {
	if (ROM->INES_Version <2) EMU->SetCHRRAMSize(32768);

	ROM->ChipRAMData =ROM->PRGROMData;
	ROM->ChipRAMSize =ROM->PRGROMSize;
	flash =new FlashROM(0xBF, 0xB7, ROM->ChipRAMData, ROM->ChipRAMSize, 4096);

	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		wcsncpy_s(nameStem, 1024, ROM->Filename, wcslen(ROM->Filename) -4);	
		reg =0xFF;
	}
	serialCount =-1;
	dspCommands =std::queue<uint8_t>();
	sync();
	
	EMU->SetCPUReadHandler(0x5, writeOpenBus);
	EMU->SetCPUReadHandler(0x7, writeOpenBus);
	EMU->SetCPUWriteHandler(0x5, writeReg);
	EMU->SetCPUWriteHandler(0x7, writeReg);
	for (int bank =0x8; bank <=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, readFlash);
		EMU->SetCPUReadHandlerDebug(bank, readFlash);
		EMU->SetCPUWriteHandler(bank, writeFlash);
	}
}

void	MAPINT	unload (void) {
	delete flash;
	flash =NULL;
	MP3Player::freeSong();
}

void	MAPINT	cpuCycle() {
	if (flash) flash->cpuCycle();
	while (dspCommands.size() >=8) {
		TCHAR filename[1024];
		uint8_t data[8];
		for (auto& c: data) {
			c =dspCommands.front();
			dspCommands.pop();
		}
		#define COMMAND data[3]
		#define PARAM1 data[5]
		#define PARAM2 data[6]
		if (data[0] ==0x7E && data[1] ==0xFF && data[2] ==0x06 && data[4] ==0x00 && data[7] ==0xEF) switch(COMMAND) {
		case 0x01: EMU->DbgOut(L"GTMP3: Next song"); break;
		case 0x02: EMU->DbgOut(L"GTMP3: Previous Song"); break;
		case 0x03: EMU->DbgOut(L"GTMP3: Select track %d", PARAM1); break;
		case 0x04: EMU->DbgOut(L"GTMP3: Volume up"); break;
		case 0x05: EMU->DbgOut(L"GTMP3: Volume down"); break;
		case 0x06: EMU->DbgOut(L"GTMP3: Select volume %d", PARAM1); break;
		case 0x07: EMU->DbgOut(L"GTMP3: Select EQ"); break;
		case 0x08: EMU->DbgOut(L"GTMP3: Select looped track %d/%d", PARAM2, PARAM1); break;
		case 0x09: EMU->DbgOut(L"GTMP3: Select TF"); break;
		case 0x0A: EMU->DbgOut(L"GTMP3: Sleep"); break;
		case 0x0B: EMU->DbgOut(L"GTMP3: Wake up"); break;
		case 0x0C: EMU->DbgOut(L"GTMP3: Chip reset"); break;
		case 0x0D: //EMU->DbgOut(L"GTMP3: Play");
			   MP3Player::start();
		           break;
		case 0x0E: //EMU->DbgOut(L"GTMP3: Pause");
		           MP3Player::stop();
			   break;
		case 0x0F: EMU->DbgOut(L"GTMP3: Select file %d/%d", PARAM1, PARAM2); break;
		case 0x12: //EMU->DbgOut(L"GTMP3: Select MP3 folder file %d", PARAM2);
 			   _snwprintf_s (filename, 1024, 1023, L"%s/MP3/%04d.mp3", nameStem, PARAM2);
			   MP3Player::loadSong(filename);
			   MP3Player::start();
		           break;
		case 0x13: EMU->DbgOut(L"GTMP3: Advert %d", PARAM1 | PARAM2 <<8); break;
		case 0x16: //EMU->DbgOut(L"GTMP3: Stop");
			   MP3Player::stop();
			   MP3Player::setPosition(0);
		           break;
		case 0x17: EMU->DbgOut(L"GTMP3: Play folder loop %d", PARAM2); break;
		case 0x18: EMU->DbgOut(L"GTMP3: Shuffle play"); break;
		case 0x19: EMU->DbgOut(L"GTMP3: Single loop enable %d", data[1]); break;
		case 0x22: EMU->DbgOut(L"GTMP3: Play %d with volume %d", PARAM1, PARAM2); break;
		default:   EMU->DbgOut(L"GTMP3: Unknown command $%02X %d %d", COMMAND, PARAM1, PARAM2); break;
		} else
			EMU->DbgOut(L"GTMP3: Invalid data %02X %02X %02X %02X %02X %02X %02X %02X", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, reg);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int	MAPINT	mapperSnd (int cycles) {
	int result =0;
	while (cycles--) result += MP3Player::getNextSample();
	return result /3;
}

uint16_t mapperNum =111;
MapperInfo MapperInfo_111 ={
	&mapperNum,
	_T("GTROM"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	saveLoad,
	mapperSnd,
	NULL
};
} // namespace
