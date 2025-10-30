#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"

namespace {
class SerialROM: public SerialDevice {
	int      bitPosition;
	uint8_t  command;
const	uint8_t* rom;
public:
	SerialROM(const uint8_t* _rom):
		bitPosition(0),
		command(0),
		rom(_rom) {
	}
	void setPins(bool select, bool newClock, bool newData) {
		if (select)
			state =0;
		else
		if (!clock && newClock) {
			if (state <8) {
				command =command <<1 | newData*1;
				if (++state ==8 && command !=0x30)
					state =0;
				else
					bitPosition =0;
			} else {
				output =rom[bitPosition >>3] >>(7 -(bitPosition &7)) &1? true: false;
				if (++bitPosition >=256 *8) state =0;
			}
		}
		clock =newClock;
	}
};

SerialROM* serialROM =NULL;

void	sync () {
	OneBus::syncPRG(0x0FFF, 0);
	OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	OneBus::load(sync);
	if (ROM->MiscROMSize &0x0100) serialROM =new SerialROM(ROM->MiscROMData +ROM->MiscROMSize -0x0100);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	OneBus::reset(resetType);
	if (serialROM) {
		serialROM->reset();
		OneBus::gpio[2].attachSerialDevice({ serialROM, 4, 3, 5 });
	}
}

void	MAPINT	unload () {
	if (serialROM) {
		delete serialROM;
		serialROM =NULL;
	}
}
uint16_t mapperNum =426;
} // namespace


MapperInfo MapperInfo_426 = {
	&mapperNum,
	_T("VT369 with serial ROM"),
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
