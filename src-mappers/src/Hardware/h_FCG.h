#pragma once

#include	"..\interface.h"
#include	"h_EEPROM.h"

enum struct FCGType {
	Unknown,
	FCG,
	LZ93D50
};

namespace FCG {
extern	uint8_t		prg;
extern	uint8_t		chr[8];
extern	uint8_t		mirroring;
extern	uint16_t	counter;
extern	uint16_t	latch;
extern	bool		irqEnabled;
extern	bool		wramEnabled;
extern	EEPROM_I2C*	eeprom;

void	syncPRG (int, int);
void	syncCHR (int, int);
void	syncMirror (void);
void	MAPINT	writeASIC (int, int, int);
BOOL	MAPINT	load (FSync, FCGType, EEPROM_I2C*);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	unload (void);
void	MAPINT	cpuCycle (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
}