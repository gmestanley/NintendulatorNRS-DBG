#include	"h_VRC3.h"

#define irqRepeat  (irq &1)
#define	irqEnabled (irq &2)
#define irqShort   (irq &4)

namespace VRC3 {
FSync		sync;
uint8_t		prg;
uint8_t		irq;
uint16_t	counter;
uint16_t	latch;

void	syncPRG (int AND, int OR) {
	EMU->SetPRG_RAM8(0x6, 0);
	EMU->SetPRG_ROM16(0x8, prg  &AND |OR);
	EMU->SetPRG_ROM16(0xC, 0xFF &AND |OR);
}

void	syncCHR (int AND, int OR) {
	EMU->SetCHR_RAM8(0, 0 &AND |OR);
}

void	MAPINT	writeCounter (int bank, int addr, int val) {
	val &=0xF;
	int shift =bank <<2 &0xC;
	latch =latch &~(0xF <<shift) | val <<shift;
}

void	MAPINT	writeIRQ (int bank, int addr, int val) {
	irq =val;
	if (irqEnabled) counter =latch;
	EMU->SetIRQ(1);
}

void	MAPINT	writeACK (int bank, int addr, int val) {
	irq =irq &~2 | irq <<1 &1;
	EMU->SetIRQ(1);
}

void	MAPINT	writePRG (int bank, int addr, int val) {
	prg =val;
	sync();
}

void	MAPINT	load (FSync _sync) {
	sync =_sync;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		prg =0;
		irq =0;
		counter =0;
		latch =0;
	}
	sync();
	
	for (int bank=0x8; bank <=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeCounter);
	EMU->SetCPUWriteHandler(0xC, writeIRQ);
	EMU->SetCPUWriteHandler(0xD, writeACK);
	EMU->SetCPUWriteHandler(0xF, writePRG);
}

void	MAPINT	cpuCycle (void) {
	if (irqEnabled) {
		int mask =irqShort? 0xFF: 0xFFFF;
		if ((counter &mask) ==mask) {
			counter =latch;
			EMU->SetIRQ(0);
		} else
			++counter;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	SAVELOAD_BYTE(stateMode, offset, data, prg);
	SAVELOAD_BYTE(stateMode, offset, data, irq);
	SAVELOAD_WORD(stateMode, offset, data, counter);
	SAVELOAD_WORD(stateMode, offset, data, latch);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace VRC3
