#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"
#include "..\..\Hardware\h_FlashROM.h"
#include "..\..\Hardware\Sound\s_MP3.h"

namespace {
FlashROM *flash =NULL;
TCHAR nameStem[1024];

struct FCArduinoMP3Kit {
	uint32_t count;
	uint8_t state;
	uint8_t command;
	uint8_t volume;
	uint8_t eq;
	uint8_t folderNumber;
} mp3Player;

void sync (void) {
	MMC3::syncPRG(0x3F, 0);
	EMU->SetPRG_ROM8(0x8, 0x00);
	EMU->SetPRG_ROM8(0xE, 0x30);
	MMC3::syncCHR(0xFF, 0);
	MMC3::syncMirror();
}

int MAPINT readFlash (int bank, int addr) {
	return flash->read(bank, addr);
}

int MAPINT readMP3Controller (int bank, int addr) {
	TCHAR filename[1024];
	int result =0xFF;
	switch(mp3Player.state) {
		case 0:	mp3Player.command =0xFF; mp3Player.count =1000; break;
		case 1:	mp3Player.command &=0xFE; result =0xFF; break;
		case 2:	mp3Player.command &=0xFE; result =0xA0; break;
		case 3:	mp3Player.command &=0xFE; result =0x20; break;
		case 4:	mp3Player.command &=0xFE; result =mp3Player.eq <<4; break;
		case 5:	mp3Player.command &=0xFE; result =mp3Player.volume <<4; break;
		case 6:	mp3Player.command &=0xFE; result =mp3Player.volume &0xF0; break;
		case 7:	mp3Player.command &=0xFE; result =mp3Player.folderNumber <<4; break;
		case 8:	mp3Player.command &=0xFE; result =0xFF; break;
		case 9: //EMU->DbgOut(L"Received command: %02X", mp3Player.command);
			mp3Player.state =0;
			if (mp3Player.command ==0x00)
				EMU->DbgOut(L"Return data");
			else
			if (mp3Player.command >=0x01 && mp3Player.command <=0xBF) {
				_snwprintf_s (filename, 1024, 1023, L"%s/%02d/%03d.mp3", nameStem, mp3Player.folderNumber +1, mp3Player.command);
				EMU->DbgOut(L"Play song %03d in folder %02d (%s)", mp3Player.command, mp3Player.folderNumber +1, filename);
				MP3Player::loadSong(filename);
				MP3Player::start();
			} else
			if (mp3Player.command >=0xC0 && mp3Player.command <=0xDF) {
				mp3Player.volume =mp3Player.command -0xC0;
				EMU->DbgOut(L"Set volume to %d", mp3Player.volume);
			} else
			if (mp3Player.command >=0xE0 && mp3Player.command <=0xEF) {
				mp3Player.eq =mp3Player.command -0xE0;
				EMU->DbgOut(L"Set EQ to %d", mp3Player.eq);
			} else
			if (mp3Player.command >=0xF0 && mp3Player.command <=0xFC) {
				mp3Player.folderNumber =mp3Player.command -0xF0;
				EMU->DbgOut(L"Set folder to %02d", mp3Player.folderNumber +1);
			} else
			if (mp3Player.command ==0xFD)
				EMU->DbgOut(L"Retrieve settings");
			else
			if (mp3Player.command ==0xFF) {
				EMU->DbgOut(L"Reset/silence");
				MP3Player::stop();
			} else
				EMU->DbgOut(L"Unknown command %02X", mp3Player.command);
			return 0xFF;
	}
	
	//EMU->DbgOut(L"Read, state %d, command %02X", mp3Player.state, mp3Player.command);
	return result;
}

void MAPINT writeFlash (int bank, int addr, int val) {
	switch (bank &~1) {
		case 0xA:
			MMC3::writeMirroringWRAM(0xA, 0, addr &1);
			break;
		case 0xC:
			MMC3::writeIRQConfig(0xC, 0, (addr &0xFF) -1);
			MMC3::writeIRQConfig(0xC, 1, 0);
			MMC3::writeIRQEnable(0xE, addr ==0xFF? 0: 1, 0);
			break;
		case 0xE:
			addr =addr <<2 &8 | addr &1;
			MMC3::writeReg(0x8, 0, 0x40); MMC3::writeReg(0x8, 1, addr <<3 |0);
			MMC3::writeReg(0x8, 0, 0x41); MMC3::writeReg(0x8, 1, addr <<3 |2);
			MMC3::writeReg(0x8, 0, 0x42); MMC3::writeReg(0x8, 1, addr <<3 |4);
			MMC3::writeReg(0x8, 0, 0x43); MMC3::writeReg(0x8, 1, addr <<3 |5);
			MMC3::writeReg(0x8, 0, 0x44); MMC3::writeReg(0x8, 1, addr <<3 |6);
			MMC3::writeReg(0x8, 0, 0x45); MMC3::writeReg(0x8, 1, addr <<3 |7);
			MMC3::writeReg(0x8, 0, 0x46); MMC3::writeReg(0x8, 1, 0x20 | addr);
			MMC3::writeReg(0x8, 0, 0x47); MMC3::writeReg(0x8, 1, 0x10 | addr);
			break;
	}
	flash->write(bank, addr, val);
}

BOOL MAPINT load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, readMP3Controller, NULL);
	ROM->ChipRAMData =ROM->PRGROMData;
	ROM->ChipRAMSize =ROM->PRGROMSize;
	flash =new FlashROM(0x37, 0x86, ROM->ChipRAMData, ROM->ChipRAMSize, 65536, 0x555, 0x2AA); // AMIC A29040B
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		wcsncpy_s(nameStem, 1024, ROM->Filename, wcslen(ROM->Filename) -4);	
		mp3Player.folderNumber =0;
		mp3Player.eq =0;
		mp3Player.volume =30;
	}
	MMC3::reset(resetType);
	for (int bank =0x8; bank<=0xF; bank++) {
		EMU->SetCPUReadHandler(bank, readFlash);
		EMU->SetCPUReadHandlerDebug(bank, readFlash);
		EMU->SetCPUWriteHandler(bank, writeFlash); 
	}
}

void MAPINT unload (void) {
	delete flash;
	flash =NULL;
	MP3Player::freeSong();
}

void MAPINT cpuCycle() {
	if (mp3Player.count && !--mp3Player.count) {
		if (++mp3Player.state <9) {
			mp3Player.count =1000;
			mp3Player.command =mp3Player.command <<1 |1;
		}
		//EMU->DbgOut(L"expire, state %d, command %02X", mp3Player.state, mp3Player.command);
	}
	MMC3::cpuCycle();
	if (flash) flash->cpuCycle();
}

int	MAPINT	mapperSnd (int cycles) {
	int result =0;
	while (cycles--) result += MP3Player::getNextSample();
	return result /3;
}

uint16_t mapperNum =451;
} // namespace

MapperInfo MapperInfo_451 ={
	&mapperNum,
	_T("Impact Soft C-IM2-BASE"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	MMC3::ppuCycle,
	MMC3::saveLoad,
	mapperSnd,
	NULL
};