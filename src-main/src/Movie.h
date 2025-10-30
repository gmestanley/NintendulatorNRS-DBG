#pragma once

#define	MOV_PLAY	0x01
#define	MOV_RECORD	0x02
#define	MOV_REVIEW	0x04

namespace Movie
{
extern unsigned char	Mode;
extern unsigned char	ControllerTypes[4];

void		ShowFrame	(void);
void		Play		(void);
void		Record		(void);
void		Stop		(void);
unsigned char	LoadInput	(void);
void		SaveInput	(unsigned char);
int		Save		(FILE *);
int		Load		(FILE *, int ver);
} // namespace Movie
