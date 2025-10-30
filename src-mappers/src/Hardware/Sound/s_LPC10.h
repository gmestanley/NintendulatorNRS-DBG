#pragma once
#include <queue>
#include "Butterworth.h"

#define LPC_FIFO_LENGTH 128

class LPC10 {
public:
	struct Codec {
		const char *name;
		int volume; // percent
		unsigned int energyBits;
		unsigned int pitchBits;
		unsigned int kBits[10];
		const short pitchMax;
		const short unvoicedChirp;
		const short *energyTable;
		const short *pitchTable;
		const short *chirpTable;
		const short *kTable[10];
	};
	struct SelectableCodec {
		std::vector<uint8_t> identifier;
		Codec *codec;
	};
	struct Variant {
		Codec *defaultCodec;
		std::vector<SelectableCodec> selectableCodecs;
		int endCommandLength;
		int endCommandNumber;
	};
private:
	struct Frame {
		short energy;
		short pitch;
		short k[10];
	};
	Codec* codec;
	const Variant* variant;
	Frame framePrev, frameCurr, frameNext;
	bool needInterpolation;
	short randomSeed;
	short currentPitch;
	short sampleIndex;
	short filterX[11];
	int16_t output;
	std::queue<bool> fifo;

	// State machine
	int chipCount, waitCycles;
	short state, returnState, gotBits, getFrameRepeat;
	Frame *getFrameDst, *getFrameSrc;

	// Host interface
	const int32_t chipClock;
	const int32_t hostClock;
	LPF_RC lowPassFilter;

	void reloadPitch();
	int filter(int);
	int rng();
	bool haveBits(unsigned int);
public:
	LPC10(Variant*, int32_t, int32_t);
	void reset();
	void clearFIFO();
	void run();
	void writeBitsLSB(int, int);
	void writeBitsMSB(int, int);
	bool getReadyState() const;
	bool getPlayingState() const;
	bool getFinishedState() const;
	int getAudio(int);
	int saveLoad (STATE_TYPE, int, unsigned char *);
};

extern LPC10::Variant lpcGeneric;
extern LPC10::Variant lpcSubor;
extern LPC10::Variant lpcBBK;
extern LPC10::Variant lpcABM;
