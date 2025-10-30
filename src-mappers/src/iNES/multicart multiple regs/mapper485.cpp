#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
uint8_t outerBank;

void sync (void) {
	if (Latch::data &0x20) {
		EMU->SetPRG_ROM16(0x8, Latch::data &0x1F | outerBank);
		EMU->SetPRG_ROM16(0xC, Latch::data &0x1F | outerBank);
	} else
		EMU->SetPRG_ROM32(0x8,(Latch::data &0x1F | outerBank) >>1);
	EMU->SetCHR_RAM8(0x0, 0x0);

	switch (Latch::data >>6) {
		case 0: EMU->Mirror_S0(); break;
		case 1: EMU->Mirror_V (); break;
		case 2: EMU->Mirror_H (); break;
		case 3: EMU->Mirror_S1(); break;
	}
}

void MAPINT writeOuterBank (int, int, int val) {
	outerBank =val <<0x5;
	sync();
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE) {
	outerBank =0;
	Latch::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x5, writeOuterBank);
}


int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset = Latch::saveLoad_D(stateMode, offset, data);
	SAVELOAD_BYTE(stateMode, offset, data, outerBank);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t MapperNum =485;
} // namespace

MapperInfo MapperInfo_485 ={
	&MapperNum,
	_T("0359"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};

