#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_OneBus.h"
#include	"..\..\Hardware\h_EEPROM.h"

namespace {
class InverterAdder: public ParallelDevice {
	uint8_t  command;
	uint8_t  commandState;
	uint8_t  latch;
	uint8_t  clockCount;
	bool     sending;
public:
	void setPins(bool select, bool newClock, uint8_t newData) {
		if (sending && commandState >0) {
			if (newClock ^ clockToDevice) {
				latch <<=1;
				if (++commandState >8) {
					sending =false;
					command =0xFF;
					commandState =0;
				}
			}
		} else
		if (newClock) {
			latch =latch &0x0F | newData <<4 &0xF0;
		} else {
			latch =latch &0xF0 | newData &0x0F;
			if (command ==0xFF && latch ==0x55) {
				command =latch;
				EMU->DbgOut(L"Received 55");
			} else
			if (command ==0x55 && latch ==0xAA) {
				EMU->DbgOut(L"Received 55 AA: Reset");
				reset();
			} else
			if (command ==0xFF && latch ==0x00) {
				//EMU->DbgOut(L"Command 0: start");
				command =0x00;
				commandState =0;
			} else
			if (command ==0xFF && latch ==0x02) {
				//EMU->DbgOut(L"Command 2: start");
				command =0x02;
				commandState =0;
			} else
			if (command ==0x02 && commandState ==0) {
				//EMU->DbgOut(L"Command 2: received byte %02X, outputting byte %02X", latch, (~latch +0xB0) &0xFF);
				sending =true;
				commandState =1;
				latch =~latch +0xB0;
			} 
			if (command ==0x00 && commandState ==0) {
				//EMU->DbgOut(L"Command 0: sending controller state");
				sending =true;
				commandState =1;
				EMU->WriteAPU(0x4, 0x016, 0x01);
				EMU->WriteAPU(0x4, 0x016, 0x00);
				for (int i =0; i <8; i++) latch =latch <<1 | EMU->ReadAPU(0x4, 0x016) &1;
			}
		}
		clockToDevice =newClock;
		dataFromDevice =latch >>7 &1;
	}
	bool getClock() {
		return clockFromDevice =!clockFromDevice;
	}
	void reset() {
		command =0xFF;
		commandState =0;
		latch =0xFF;
		sending =false;
	}
};

InverterAdder inverterAdder;

void	sync () {
	OneBus::syncPRG(0x0FFF, 0);
	OneBus::syncCHR(0x7FFF, 0);
	OneBus::syncMirror();
}

BOOL	MAPINT	load (void) {
	OneBus::load(sync);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	OneBus::reset(resetType);
	OneBus::gpio[1].attachParallelDevice({&inverterAdder, 255, 2, 1, 4, 0x0F, 3, 0x01 });
	inverterAdder.reset();
}

uint16_t mapperNum =496;
} // namespace


MapperInfo MapperInfo_496 = {
	&mapperNum,
	_T("VT369 with inverteradder"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	OneBus::cpuCycle,
	OneBus::ppuCycle,
	OneBus::saveLoad,
	NULL,
	NULL
};
