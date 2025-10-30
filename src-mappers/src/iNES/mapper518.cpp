#include "..\DLL\d_iNES.h"
#include "..\Hardware\Sound\s_LPC10.h"
#include "..\Hardware\simplefdc.hpp"

namespace {
uint8_t chr;
uint8_t reg[2];
LPC10 lpc(&lpcSubor, 800000, 1789773);
FDC fdc;
uint8_t secondaryRAM[128*1024];

bool isSB97 (){
	return ROM->dipValue == 97;
}

int MAPINT trapNTRead (int bank, int addr) {
	if (addr <0x3C0) { // CHR A12 is CIRAM A10 during reads from PPU $0000-$0FFF
		uint8_t newCHR = bank >>(reg[1] &1) &1;
		if (newCHR != chr) EMU->SetCHR_RAM4(0x0, chr = newCHR);
	}
	return EMU->ReadCHR(bank, addr);
}

void MAPINT trapCHRWrite (int bank, int addr, int val) {
	EMU->SetCHR_RAM8(0x0, 0); // CHR A12 is always 0 during writes to PPU $0000-$0FFF
	EMU->WriteCHR(bank, addr, val);
}

void sync (void) {
	EMU->SetPRG_RAM8(0x6, 0);
	if (reg[0] &0x80) {
		if (reg[1] &0x04)
			for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetPRG_Ptr4(bank, &secondaryRAM[reg[0] <<15 &0x18000 | bank <<12 &0x7000], TRUE);
		else {
			for (int bank = 0x8; bank <= 0xB; bank++) EMU->SetPRG_Ptr4(bank, &secondaryRAM[reg[0] <<14 &0x1C000 | bank <<12 &0x3000], TRUE);
			EMU->SetPRG_ROM16(0xC, 0);
		}
	} else {
		if (reg[1] &0x04)
			EMU->SetPRG_ROM32(0x8, reg[0]);
		else {
			EMU->SetPRG_ROM16(0x8, reg[0]);
			EMU->SetPRG_ROM16(0xC, 0);
		}
	}
	EMU->SetCHR_RAM4(0x0, reg[1] &0x02? chr: 0);
	EMU->SetCHR_RAM4(0x4, 1);
	if (reg[1] &0x01)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
	for (int bank =0; bank <= 3; bank++) EMU->SetPPUWriteHandler    (bank, reg[1] &0x02? trapCHRWrite: EMU->WriteCHR);
	for (int bank =8; bank <=11; bank++) EMU->SetPPUReadHandler     (bank, reg[1] &0x02? trapNTRead: EMU->ReadCHR);
	for (int bank =8; bank <=11; bank++) EMU->SetPPUReadHandlerDebug(bank, EMU->ReadCHR);
}

int MAPINT readReg (int bank, int addr) {
	switch (addr &0xF00) {
		case 0x300:
			return lpc.getFinishedState()? 0x8F: lpc.getReadyState()? 0x80: 0x00;
		case 0x600:
			return isSB97()? fdc.readIO(addr): *EMU->OpenBus;
		default:
			return *EMU->OpenBus;
	}
}

void MAPINT writeReg (int bank, int addr, int val) {
	switch (addr &0xF00) {
		case 0x000:
			reg[0] =val;
			sync();
			break;
		case 0x200:
			reg[1] =val;
			sync();
			break;
		case 0x300:
			lpc.writeBitsLSB(8, val);
			break;
		case 0x500:
			if (addr ==0x501) addr =2; else
			if (addr ==0x500) addr =7;
			if (isSB97()) fdc.writeIO(addr, val);
			break;
	}
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg[0] =0;
	reg[1] =0;
	sync();
	if (isSB97()) fdc.reset();
	lpc.reset();
	(EMU->GetCPUWriteHandler(0x4))(0x4, 0x017, 0x40); // For some reason, the SB-97 never disables the Frame IRQ.
	EMU->SetCPUReadHandler(0x5, readReg);
	EMU->SetCPUWriteHandler(0x5, writeReg);
}

void MAPINT cpuCycle (void) {
	lpc.run();
	if (isSB97()) {
		if (ROM->changedDisk35) {
			fdc.ejectDisk(0);
			if (ROM->diskData35 !=NULL) fdc.insertDisk(0, &(*ROM->diskData35)[0], ROM->diskData35->size(), true, &ROM->modifiedDisk35);
			ROM->changedDisk35 =false;
		}
		fdc.run();
		if (fdc.irqRaised())
			EMU->SetIRQ(0);
		else
			EMU->SetIRQ(1);
	}
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	offset =lpc.saveLoad(stateMode, offset, data);
	if (isSB97()) {
		for (auto& c: secondaryRAM) SAVELOAD_BYTE(stateMode, offset, data, c);
		//offset =fdc.saveLoad(stateMode, offset, data); // no saveLoad implemented yet
	}
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

static int MAPINT mapperSound (int cycles) {
	return lpc.getAudio(cycles);
}

uint16_t mapperNum =518;
} // namespace

MapperInfo MapperInfo_518 = {
	&mapperNum,
	_T("小霸王 SB97"),
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

/*
	480F Printer control?

*/