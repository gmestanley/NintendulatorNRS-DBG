#pragma once

#include	"..\interface.h"

namespace H3001 {
extern	uint8_t		prgInvert;
extern	uint8_t		prg[2];
extern	uint8_t		chr[8];
extern	uint8_t		mirroring;
extern	uint8_t		irq;
extern	uint16_t	counter;
extern	uint16_t	latch;
extern	FSync		sync;

void	syncPRG (int, int);
void	syncCHR (int, int);
void	syncMirror ();
void	MAPINT	writePRG (int, int, int);
void	MAPINT	writeMisc (int, int, int);
void	MAPINT	writeCHR (int, int, int);
void	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	cpuCycle (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);

} // namespace H3001
