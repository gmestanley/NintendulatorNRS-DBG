#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\h_VRC24.h"

namespace {
uint8_t		reg[5]; // Fourth register is the CNROM latch

void	sync (void) {
	// 5003.0-2: PRG size (1 MiB->8 KiB, although 1 MiB and 512 KiB are not connected and so function as 256 KiB) and mode (>=64 KiB: MMC3, <64 KiB: NROM-256/128/64)
	int prgAND =0x7F >>(reg[2] &7) &0x1F;
	if (prgAND &0x04) { 
		if (reg[3] &0x10)
			VRC24::syncPRG(prgAND, reg[0] &~prgAND);
		else
			MMC3::syncPRG(prgAND, reg[0] &~prgAND);
	} else
		for (int bank =0; bank <4; bank++) EMU->SetPRG_ROM8(0x8 +bank*2, bank &prgAND | reg[0] &~prgAND);

	// 5003.3-5: CHR size (1 MiB->8 KiB, though 1 MiB and 512 KiB are not reachable with inner bank registers and so function as 256 KiB) and mode (>=64 KiB: MMC3, <64 KiB: (C)NROM-256/128/64)
	int chrAND =0x3FF >>(reg[2] >>3 &7) &0xFF;
	if (chrAND &0x20) {
		if (reg[3] &0x10)
			VRC24::syncCHR(chrAND, (reg[1] <<3) &~chrAND);
		else
			MMC3::syncCHR(chrAND, (reg[1] <<3) &~chrAND);
	} else {
		chrAND >>=3;
		EMU->SetCHR_ROM8(0x0, reg[4] &chrAND | reg[1] &~chrAND);
	}

	if (reg[3] &0x10)
		VRC24::syncMirror();
	else
		MMC3::syncMirror();
	
	if (ROM->dipValueSet && reg[0] &0xC0 && (reg[0] &0xC0) ==ROM->dipValue) for (int bank =0x8; bank <=0xF; bank++) EMU->SetPRG_OB4(bank);
}

void	MAPINT	writeLatch (int bank, int addr, int val) {
	reg[4] =val;
	sync();
}

void	applyMode (void) {
	if ((reg[2] >>3 &7) >=5) // In (C)NROM mode, $8000-$FFFF reaches only the CNROM latch
		for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, writeLatch);
	else
	if (reg[3] &0x10) {
		VRC24::A0 =reg[3] &1? 0xA: 0x5;
		VRC24::A1 =reg[3] &1? 0x5: 0xA;
		for (int bank =0x8; bank <=0xF; bank++) EMU->SetCPUWriteHandler(bank, VRC24::write);
	} else {
		EMU->SetCPUWriteHandler(0x8, MMC3::writeReg);
		EMU->SetCPUWriteHandler(0x9, MMC3::writeReg);
		EMU->SetCPUWriteHandler(0xA, MMC3::writeMirroringWRAM);
		EMU->SetCPUWriteHandler(0xB, MMC3::writeMirroringWRAM);
		EMU->SetCPUWriteHandler(0xC, MMC3::writeIRQConfig);
		EMU->SetCPUWriteHandler(0xD, MMC3::writeIRQConfig);
		EMU->SetCPUWriteHandler(0xE, MMC3::writeIRQEnable);
		EMU->SetCPUWriteHandler(0xF, MMC3::writeIRQEnable);
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	if (~reg[3] &0x20) {
		reg[addr &3] =val;
		if ((addr &3) ==3) applyMode();
		sync();
	}
}

BOOL	MAPINT	load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, NULL, NULL, NULL);
	VRC24::load(sync, true, 0x01, 0x02, NULL, true, 0);
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	for (auto& c: reg) c =0;
	MMC3::reset(resetType);	
	VRC24::reset(RESET_HARD);
	EMU->SetCPUWriteHandler(0x5, writeReg);	
	applyMode();
}

void	MAPINT	cpuCycle (void) {
	if (reg[3] &0x10)
		VRC24::cpuCycle();
	else
		MMC3::cpuCycle();
}

void	MAPINT	ppuCycle (int addr, int scanline, int cycle, int isRendering) {
	if (~reg[3] &0x10) MMC3::ppuCycle(addr, scanline, cycle, isRendering);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	offset =VRC24::saveLoad(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) {
		applyMode();
		sync();
	}
	return offset;
}

uint16_t mapperNum =445;
} // namespace

MapperInfo MapperInfo_445 ={
	&mapperNum,
	_T("DG574B"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	ppuCycle,
	saveLoad,
	NULL,
	NULL
};