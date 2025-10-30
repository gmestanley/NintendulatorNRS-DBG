#pragma once
#include	"..\interface.h"

enum struct MMC3Type {
	NEC,
	Sharp,
	Acclaim,
	MMC6,
	AX5202P,
	FCEUX
};

namespace MMC3 {
extern	MMC3Type	type;
extern	uint8_t		index;
extern	uint8_t		reg[8];
extern	uint8_t		mirroring;
extern	uint8_t		wramControl;

extern	uint8_t		reloadValue;
extern	uint8_t		counter;
extern	uint8_t		prescaler;
extern	bool		reload;
extern	bool		enableIRQ;
extern	uint8_t		pa12Filter;
extern	uint16_t	prevAddr;

extern	FSync		sync;

void	MAPINT	load			(FSync, MMC3Type, int (*) (int), int (*)(int), FCPURead, FCPUWrite);
void	MAPINT	reset			(RESET_TYPE);
void		syncMirror		(void);
void		syncMirrorA17		(void);
int		getPRGBank		(int);
void		syncPRG			(int,int);
void		syncPRG_RAM		(int,int);
void		syncPRG			(int,int);
void		syncWRAM		(int);
int		getCHRBank		(int);
void		syncCHR			(int,int);
void		syncCHR_ROM		(int,int);
void		syncCHR_RAM		(int,int);
void	MAPINT	write			(int,int,int);
void	MAPINT	writeReg		(int,int,int);
void	MAPINT	writeMirroringWRAM	(int,int,int);
void	MAPINT	writeIRQConfig		(int,int,int);
void	MAPINT	writeIRQEnable		(int,int,int);
void	MAPINT	cpuCycle		(void);
void	MAPINT	ppuCycle		(int,int,int,int);
int	MAPINT	saveLoad		(STATE_TYPE,int,unsigned char *);
} // namespace MMC3
