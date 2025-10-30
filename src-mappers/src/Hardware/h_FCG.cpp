#include	"h_FCG.h"
#include	"../DLL/d_iNES.h"

namespace FCG {
uint8_t		prg;
uint8_t		chr[8];
uint8_t		mirroring;
uint16_t	counter;
uint16_t	latch;
bool		irqEnabled;
bool		wramEnabled;

FSync		sync;
FCGType		type;
EEPROM_I2C*	eeprom;

void	syncPRG (int AND, int OR) {
	EMU->SetPRG_ROM16(0x8, prg  &AND |OR);
	EMU->SetPRG_ROM16(0xC, 0xFF &AND |OR);
	if (wramEnabled || ROM->MiscROMSize)
		EMU->SetPRG_RAM8(0x6, 0);
	else
		for (int bank =0x6; bank<=0x7; bank++) EMU->SetPRG_OB4(bank);
}	

void	syncCHR (int AND, int OR) {
	for (int bank =0; bank <8; bank++) EMU->SetCHR_ROM1(bank, chr[bank] &AND |OR);
}

void	syncMirror (void) {
	switch (mirroring) {
		case 0:	EMU->Mirror_V(); break;
		case 1:	EMU->Mirror_H(); break;
		case 2:	EMU->Mirror_S0(); break;
		case 3:	EMU->Mirror_S1(); break;
	}
}

int	MAPINT	readEEPROM (int bank, int addr) {
	if (eeprom)
		return (eeprom->getData() *0x10) | (*EMU->OpenBus &~0x10);
	else	
		return *EMU->OpenBus;
}

void	MAPINT	writeASIC (int bank, int addr, int val) {
	switch (addr &0xF) {
		case 0x0: case 0x1: case 0x2: case 0x3:	case 0x4: case 0x5: case 0x6: case 0x7:
			chr[addr &0x7] =val;
			break;
		case 0x8:
			prg =val;
			break;
		case 0x9:
			mirroring =val &3;
			break;
		case 0xA:
			irqEnabled =!!(val &1);
			if (type !=FCGType::FCG) counter =latch;
			if (irqEnabled && !counter)
				EMU->SetIRQ(0);
			else
				EMU->SetIRQ(1);
			break;
		case 0xB:
			if (type ==FCGType::FCG)
				counter =counter &0xFF00 | val;
			else
				latch =latch &0xFF00 | val;
			break;
		case 0xC:
			if (type ==FCGType::FCG)
				counter =counter &0x00FF | val <<8;
			else
				latch =latch &0x00FF | val <<8;
			break;
		case 0xD:
			if (eeprom)
				eeprom->setPins(false, !!(val &0x20), !!(val &0x40));
			else
			if (type !=FCGType::FCG)
				wramEnabled =!!(val &0x20);
			break;
	}
	sync();
}

BOOL	MAPINT	load (FSync _sync, FCGType _type, EEPROM_I2C* _eeprom) {
	sync =_sync;
	type =_type;
	eeprom =_eeprom;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0x00;
		for (int bank =0; bank <8; bank++) chr[bank] =bank;
		counter =0;
		latch =0;
		irqEnabled =false;
		wramEnabled =false;
	}
	sync();
	
	if (eeprom) for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUReadHandler(bank, readEEPROM);
	for (int bank =(type ==FCGType::LZ93D50? 0x8: 0x6); bank <=(type ==FCGType::FCG? 0x7: 0xF); bank++) EMU->SetCPUWriteHandler(bank, writeASIC);
}

void	MAPINT	unload (void) {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

void	MAPINT	cpuCycle (void) {
	if (irqEnabled && !--counter) EMU->SetIRQ(0);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, mirroring);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	if (type !=FCGType::FCG) SAVELOAD_WORD(stateMode, offset, data, latch);
	SAVELOAD_BOOL(stateMode, offset, data, irqEnabled);
	if (type !=FCGType::FCG) SAVELOAD_BOOL(stateMode, offset, data, wramEnabled);
	if (eeprom) offset =eeprom->saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace FCG