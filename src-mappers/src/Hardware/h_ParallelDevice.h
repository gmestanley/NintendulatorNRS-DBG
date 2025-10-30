#pragma once
#include	"..\interface.h"

class ParallelDevice {
protected:
	uint8_t    state;
	bool       clockToDevice;
	bool       clockFromDevice;
	uint8_t    dataToDevice;
	uint8_t    dataFromDevice;
public:
	           ParallelDevice();
virtual	void       reset();
virtual	bool       getClock();
virtual	uint8_t    getData() const;
virtual	void       setPins(bool, bool, uint8_t);
virtual	int MAPINT saveLoad (STATE_TYPE, int, unsigned char *);
};
