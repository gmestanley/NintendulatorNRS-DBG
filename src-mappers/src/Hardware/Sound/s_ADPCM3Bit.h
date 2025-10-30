#pragma once
#include <stdint.h>
#include <queue>
#include <vector>
#include "../../interface.h"
#include "Butterworth.h"

class ADPCM3Bit {
	uint32_t             count;         // counter until next decoded sample is output
	uint32_t             period;        // counter target until next decoded sample is output
	uint32_t             inhibit;       // counter after buffer full to inhibit acknowledgement
	uint8_t	             command;       // current command
	uint8_t              latch;         // latch for loading the next command
	uint8_t              data;          // 4-bit data
	uint8_t              bytesLeft;     // bytes left to receive for the current command
	bool                 clock;         // data clock signal from host
	bool                 ready;         // indicates that frame buffer is not full
	bool		     playing;       // whether the playback buffer is being cnoused
	uint8_t              index;         // index into ADPCM table
	int8_t               output;        // current decoded ADPCM output
	std::vector<uint8_t> input;         // buffer of input bytes
	std::queue<int64_t>  frames;        // buffer of input frames, at most 12
	int                  chipCount;     // to convert from host to chip clock
	const int32_t        chipClock;     // "
	const int32_t        hostClock;     // "
	LPF_RC               lowPassFilter;
	void                 decodeSample(uint8_t);
public:
		   ADPCM3Bit (int32_t, int32_t);
	void       reset     ();
	void       run       ();
	int MAPINT getAudio  (int);
        bool       getAck    ();
	bool       getReady  () const;
	void       setClock  (bool);
	void       setData   (uint8_t);
	int MAPINT saveLoad  (STATE_TYPE, int, unsigned char *);
};