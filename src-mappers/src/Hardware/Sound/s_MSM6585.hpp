#pragma once
#include "..\..\interface.h"
#include "Butterworth.h"

class MSM6585 {
	bool whichNibble;
	uint8_t input;
	int16_t signal;
	int32_t count;
	int32_t hostClock;
	int32_t rate;
	int16_t step;
	Butterworth lowPassFilter; 
	int (*getInput)(void);
public:
	MSM6585 (int32_t, int (*)(void));
	void reset (void);
	void setRate (uint8_t);
	void run (void);
	int32_t getOutput (void);
	int MAPINT saveLoad (STATE_TYPE, int, unsigned char *);
};
