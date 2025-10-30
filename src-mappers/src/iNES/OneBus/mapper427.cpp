#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"
#include	"..\..\Hardware\h_EEPROM.h"

namespace {
class Inverter: public SerialDevice {
	uint8_t  command;
     	uint8_t  result;
public:
	void setPins(bool select, bool newClock, bool newData) {
		if (clock && newClock && data && !newData) // START in I²C
			state =1;  // Write command and data
		else
		if (clock && newClock && !data && newData) // STOP in I²C
			state =19; // Read result
		else
		if (!clock && newClock) {
			if (state ==0)
				data =newData;
			else
			if (state >=1 && state < 9) { // command byte
				command =command <<1 | newData*1;
				if (++state ==9 && command !=0x80) state =0;
			} else
			if (state ==9) // terminating bit
				state++;
			else
			if (state >=10 && state <18) { // data byte
				result =result <<1 | newData*1;
				if (++state ==18) result =-result >>4 &0xF | -result <<4 &0xF0;
			} else
			if (state ==18) // terminating bit
				state =0;
			else {
				output =!!(result &0x80);
				result <<=1;
			}
		}
		clock =newClock;
		data =newData;
	}
};

Inverter Inverter;
EEPROM_24C04 *eeprom =NULL;

void	sync () {
	OneBus::syncPRG(0x0FFF, 0);
	OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	OneBus::load(sync);
	if (ROM->INES2_PRGRAM ==0x30) eeprom =new EEPROM_24C04(0, ROM->PRGRAMData);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	OneBus::reset(resetType);
	switch(ROM->INES2_SubMapper) {
		case 0:	// MOGIS M320, The Oregon Trail
			OneBus::gpio[1].attachSerialDevice({&Inverter, 255, 5, 4 });
			if (eeprom) {
				eeprom->reset();
				OneBus::gpio[2].attachSerialDevice({eeprom, 255, 2, 3 });
			}
			break;
		case 1: // Lexibook Cyber Arcade Pocket
			OneBus::gpio[2].attachSerialDevice({&Inverter, 255, 0, 1 });	
			break;
	}		
}

void	MAPINT	unload () {
	if (eeprom) {
		delete eeprom;
		eeprom =NULL;
	}
}

uint16_t mapperNum =427;
} // namespace


MapperInfo MapperInfo_427 = {
	&mapperNum,
	_T("VT369 with inverter/EEPROM"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	OneBus::cpuCycle,
	OneBus::ppuCycle,
	OneBus::saveLoad,
	NULL,
	NULL
};
