#include	"../Dll/d_iNES.h"

namespace {
uint8_t		prg[3];
uint8_t		chr[4];
uint8_t		state[2];

void	sync () {
	EMU->SetPRG_ROM8(0x8, prg[0]);
	EMU->SetPRG_ROM8(0xA, prg[1]);
	EMU->SetPRG_ROM8(0xC, prg[2]);
	EMU->SetPRG_ROM8(0xE, 0xFF);
	iNES_SetCHR_Auto4(0, chr[state[0] |0]);
	iNES_SetCHR_Auto4(4, chr[state[1] |2]);
	switch(chr[state[0]] >>6) {
		case 0: EMU->Mirror_Custom(0, 0, 0, 1); break;
		case 1: EMU->Mirror_H(); break;
		case 2: EMU->Mirror_V(); break;
		case 3: EMU->Mirror_S1(); break;
	}
}

int	MAPINT	trapCHRRead (int bank, int addr) {
	int result =EMU->ReadCHR(bank, addr); // Switch occurs *after* byte is read
	switch (addr &(bank &4? 0x3F8: 0x3FF)) {
		case 0x3D8: state[bank >>2] =1; sync(); break; 
		case 0x3E8: state[bank >>2] =0; sync(); break; 
	}
	return result;
}

void	MAPINT	writePRG (int bank, int, int val) {
	prg[bank >>1 &3] =val;
	sync();
}

void	MAPINT	writeCHR (int, int addr, int val) {
	chr[addr >>10 &3] =val;
	sync();
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		for (auto& c: prg) c =0;
		for (auto& c: chr) c =0;
		for (auto& c: state) c =0;
	}
	sync();
	
	for (int bank =0x8; bank<=0xD; bank++) EMU->SetCPUWriteHandler(bank, writePRG);
	for (int bank =0xE; bank<=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeCHR);
	
	EMU->SetPPUReadHandler(0x3, trapCHRRead);
	EMU->SetPPUReadHandler(0x7, trapCHRRead);
	EMU->SetPPUReadHandlerDebug(0x3, EMU->ReadCHRDebug);
	EMU->SetPPUReadHandlerDebug(0x7, EMU->ReadCHRDebug);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: chr) SAVELOAD_BYTE(stateMode, offset, data, c);
	for (auto& c: state) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}
uint16_t mapperNum =495;
} // namespace

MapperInfo MapperInfo_495 ={
	&mapperNum,
	_T("N-46"),
	COMPAT_FULL,
	NULL,
	reset,
	NULL,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};
