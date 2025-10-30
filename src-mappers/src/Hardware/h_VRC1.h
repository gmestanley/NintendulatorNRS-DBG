#pragma once

#include	"..\interface.h"

namespace VRC1 {
extern	uint8_t		prg[3];
extern	uint8_t		chr[2];
extern	uint8_t		misc;
extern	FSync		sync;

void	syncPRG (int, int);
void	syncCHR (int, int);
void	syncMirror ();
void	MAPINT	writePRG (int, int, int);
void	MAPINT	writeMisc (int, int, int);
void	MAPINT	writeCHR (int, int, int);
void	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);

} // namespace VRC1
