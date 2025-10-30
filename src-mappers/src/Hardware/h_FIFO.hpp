#pragma once
#include "..\interface.h"

class FIFO {
	const size_t capacity;
	uint8_t *data;
	int16_t front;
	int16_t back;
public: 
	size_t size (void);
	bool halfFull (void);
	int retrieve (void);
	void add (uint8_t);
	void reset (void);
	FIFO (size_t);
	~FIFO ();
	int MAPINT saveLoad (STATE_TYPE, int, unsigned char *);
};
