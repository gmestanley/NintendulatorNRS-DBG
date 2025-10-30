#pragma once
#include	"..\interface.h"

namespace Latch {
extern  bool            lockable;
extern	uint8_t		data;
extern	uint8_t		dataLocked;
extern	uint16_t	addr;
extern	uint16_t	addrLocked;
extern	FSync		sync;

typedef int (*FBusConflict)(int,int);

int     busConflictAND          (int,int); // AND-type bus conflicts, all bits
int     busConflictOR           (int,int); // OR-type bus conflicts, all bits
int     busConflictROM          (int,int); // ROM always wins, all bits
void	MAPINT	write			(int,int,int);
void	MAPINT	setLockedBits	(uint16_t, uint8_t);
void	MAPINT	load		(FSync);
void	MAPINT	load		(FSync,FBusConflict);
void	MAPINT	load		(FSync,FBusConflict,bool);
void	MAPINT	reset		(RESET_TYPE);
void	MAPINT	resetHard	(RESET_TYPE);
int	MAPINT	saveLoad_AD	(STATE_TYPE,int,unsigned char *);
int	MAPINT	saveLoad_AL	(STATE_TYPE,int,unsigned char *);
int	MAPINT	saveLoad_A	(STATE_TYPE,int,unsigned char *);
int	MAPINT	saveLoad_D	(STATE_TYPE,int,unsigned char *);
} // namespace Latch
