#include	"..\interface.h"

namespace VRC3 {
void	syncPRG (int, int);
void	syncCHR (int, int);
void	MAPINT	load (FSync);
void	MAPINT	reset (RESET_TYPE);
void	MAPINT	cpuCycle (void);
int	MAPINT	saveLoad (STATE_TYPE, int, unsigned char *);
} // namespace VRC3
