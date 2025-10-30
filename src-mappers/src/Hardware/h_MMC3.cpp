#include "h_MMC3.h"

namespace MMC3 {
MMC3Type type;
uint8_t index;
uint8_t reg[8];
uint8_t mirroring;
uint8_t wramControl;

uint8_t counter;
uint8_t prescaler;
uint8_t reloadValue;
bool reload;
bool enableIRQ;
uint8_t pa12Filter;
uint16_t prevAddr;

FSync sync;
FCPURead cbWRAMRead;
FCPUWrite cbWRAMWrite;

int (*cbGetPRGBank)(int);
int (*cbGetCHRBank)(int);


void syncWRAM (int OR) {
	if (type == MMC3Type::MMC6)
		EMU->SetPRG_RAM4(0x7, 0);
	else
		EMU->SetPRG_RAM8(0x6, OR);
}

int getPRGBank (int bank) {
	if (index &0x40 && ~bank &1) bank ^=2;
	return bank &2? 0xFE | bank &1: reg[6 | bank &1];
};

void syncPRG (int AND, int OR) {
	for (int bank = 0; bank < 4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, cbGetPRGBank(bank) &AND |OR);
}

void syncPRG_RAM (int AND, int OR) {
	for (int bank = 0; bank < 4; bank++) EMU->SetPRG_RAM8(0x8 +bank*2, cbGetPRGBank(bank) &AND |OR);
}

int getCHRBank (int bank) {
	if (index &0x80) bank ^=4;
	if (bank &4)
		return reg[bank -2];
	else
		return reg[bank >>1] &~1 | bank&1;
};

void syncCHR (int AND, int OR) {
	if (ROM->CHRROMSize)
		syncCHR_ROM(AND, OR);
	else
		syncCHR_RAM(AND, OR);
}

void syncCHR_ROM (int AND, int OR) {
	for (int bank = 0; bank < 8; bank++) EMU->SetCHR_ROM1(bank, cbGetCHRBank(bank) &AND |OR);
}

void syncCHR_RAM (int AND, int OR) {
	for (int bank = 0; bank < 8; bank++) EMU->SetCHR_RAM1(bank, cbGetCHRBank(bank) &AND |OR);
}

void syncMirror (void) {
	if (mirroring &1)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void syncMirrorA17 (void) {
	for (int bank = 0; bank < 8; bank++) EMU->SetCHR_NT1(bank |8, cbGetCHRBank(bank) >>7);
}

int MAPINT wramRead_MMC3 (int bank, int addr) {
	if (wramControl &0x80 || type == MMC3Type::AX5202P || type == MMC3Type::FCEUX)
		return cbWRAMRead? cbWRAMRead(bank, addr): EMU->ReadPRG(bank, addr);
	else
		return *EMU->OpenBus;
}

void MAPINT wramWrite_MMC3 (int bank, int addr, int val) {
	if ((wramControl &0x80 || type == MMC3Type::AX5202P || type == MMC3Type::FCEUX) && ~wramControl &0x40) {
		EMU->WritePRG(bank, addr, val);
		if (cbWRAMWrite) cbWRAMWrite(bank, addr, val);
	}
}

int MAPINT wramRead_MMC6 (int bank, int addr) {
	// "If neither bank is enabled for reading, the $7000-$7FFF area is open bus.
	// If only one bank is enabled for reading, the other reads back as zero."
	addr &=0x3FF;
	if (~addr &0x200) // first 512 bytes
		return wramControl &0x20? (cbWRAMRead? cbWRAMRead(0x7, addr): EMU->ReadPRG(0x7, addr)): (wramControl &0x80? 0x00: *EMU->OpenBus);
	else // second 512 bytes
		return wramControl &0x80? (cbWRAMRead? cbWRAMRead(0x7, addr): EMU->ReadPRG(0x7, addr)): (wramControl &0x20? 0x00: *EMU->OpenBus);
}

void MAPINT wramWrite_MMC6 (int bank, int addr, int val) {
	// "The write-enable bits only have effect if that bank is enabled for reading, otherwise the bank is not writable."
	addr &=0x3FF;
	if (~addr &0x200) { // first 512 bytes
		if (wramControl &0x20 && wramControl &0x10) {
			EMU->WritePRG(0x7, addr, val);
			if (cbWRAMWrite) cbWRAMWrite(0x7, addr, val);
		}
	} else { // second 512 bytes 
		if (wramControl &0x80 && wramControl &0x40) {
			EMU->WritePRG(0x7, addr, val);
			if (cbWRAMWrite) cbWRAMWrite(0x7, addr, val);
		}
	}
}

void MAPINT write (int bank, int addr, int val) {
	switch(bank &~1) {
		case 0x8: writeReg(bank, addr, val); break;
		case 0xA: writeMirroringWRAM(bank, addr, val); break;
		case 0xC: writeIRQConfig(bank, addr, val); break;
		case 0xE: writeIRQEnable(bank, addr, val); break;
	}
}

void MAPINT writeReg (int bank, int addr, int val) {
	if (addr &1)
		reg[index &7] = val;
	else 
		index = val;
	sync();
}

void MAPINT writeMirroringWRAM (int bank, int addr, int val) {
	if (addr &1)
		wramControl = val;
	else
		mirroring = val;
	sync();
}

void MAPINT writeIRQConfig (int bank, int addr, int val) {
	if (addr &1) { 
		if (type ==MMC3Type::Acclaim) prescaler = 7;
		counter = 0;
		reload = true;
	} else
		reloadValue = val;
}

void MAPINT writeIRQEnable (int bank, int addr, int val) {
	enableIRQ =!!(addr &1);
	if (!enableIRQ) EMU->SetIRQ(1);
}

void MAPINT load (FSync _sync, MMC3Type _type, int (*_getPRGBank)(int), int (*_getCHRBank)(int), FCPURead _readWRAM, FCPUWrite _writeWRAM) {
	sync = _sync;
	type = _type;
	cbGetPRGBank = _getPRGBank? _getPRGBank: getPRGBank;
	cbGetCHRBank = _getCHRBank? _getCHRBank: getCHRBank;
	cbWRAMRead = _readWRAM; // EMU->ReadPRG is not initialized at this point.
	cbWRAMWrite = _writeWRAM;
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		index = 0x00;
		reg[0] = 0x00; reg[1] = 0x02;
		reg[2] = 0x04; reg[3] = 0x05; reg[4] = 0x06; reg[5] = 0x07;
		reg[6] = 0x00; reg[7] = 0x01;
		mirroring = type == MMC3Type::FCEUX && ~ROM->INES_Flags &1? 1: 0;
		wramControl = 0;
		enableIRQ = false;
		reload = false;
		counter = 0;
		prescaler = 7;
		pa12Filter = 0;
		prevAddr = 0;
	}
	for (int bank = 0x6; bank <= 0x7; bank++) {
		EMU->SetCPUReadHandler     (bank, type ==MMC3Type::MMC6? wramRead_MMC6:  wramRead_MMC3);
		EMU->SetCPUReadHandlerDebug(bank, type ==MMC3Type::MMC6? wramRead_MMC6:  wramRead_MMC3);
		EMU->SetCPUWriteHandler    (bank, type ==MMC3Type::MMC6? wramWrite_MMC6: wramWrite_MMC3);
	}
	
	for (int bank = 0x8; bank <= 0x9; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank = 0xA; bank <= 0xB; bank++) EMU->SetCPUWriteHandler(bank, writeMirroringWRAM);
	for (int bank = 0xC; bank <= 0xD; bank++) EMU->SetCPUWriteHandler(bank, writeIRQConfig);
	for (int bank = 0xE; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, writeIRQEnable);
	EMU->SetIRQ(1);
	sync();
}

void clockCounter () {
	uint8_t prevCounter = counter;
	counter =!counter? reloadValue: --counter;
	if ((prevCounter || reload || type==MMC3Type::Sharp) && !counter && enableIRQ) EMU->SetIRQ(0);
	reload = false;
}

void MAPINT cpuCycle (void) {
	if (type !=MMC3Type::Acclaim && pa12Filter) pa12Filter--;
}

void MAPINT ppuCycle (int addr, int scanline, int cycle, int) {
	bool hBlank = false;
	if (type ==MMC3Type::FCEUX)
		hBlank = scanline >=0 && scanline <239 && cycle ==256;
	else
	if (type ==MMC3Type::Acclaim) {
		if (prevAddr &0x1000 && ~addr &0x1000 && !(prescaler++ &7)) hBlank = true;
		prevAddr = addr;
	} else
	if (addr &0x1000) {
		if (!pa12Filter) hBlank = true;
		pa12Filter = 3;
	} 
	if (hBlank) clockCounter();
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, index);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_BYTE(stateMode, offset, data, wramControl);
	SAVELOAD_BYTE(stateMode, offset, data, counter);
	SAVELOAD_BYTE(stateMode, offset, data, reloadValue);
	SAVELOAD_BOOL(stateMode, offset, data, reload);
	SAVELOAD_BOOL(stateMode, offset, data, enableIRQ);
	if (type ==MMC3Type::Acclaim) {
		SAVELOAD_BYTE(stateMode, offset, data, prescaler);
		SAVELOAD_WORD(stateMode, offset, data, prevAddr);
	} else
		SAVELOAD_BYTE(stateMode, offset, data, pa12Filter);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace MMC3
