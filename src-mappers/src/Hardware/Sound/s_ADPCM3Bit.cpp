#include "s_ADPCM3Bit.h"

/*
	ADPCM chip of unknown name that takes 3-bit ADPCM codes in 8-byte frames that decode to 21 output samples per frame.
	A software version of the decoding algorithm can be found in the sound CPU code of most VT1682 games.
	The same decoding algorithm is also executed by the sound CPU of VT369 conversions of VT03 games that originally used this chip.
		
	The chip has a 96-byte buffer that holds the ADPCM data for twelve 8-byte frames.
	For the given "period" values to make any sense, its master clock would have to be 8/7 the NTSC subcarrier rate, or 4.09 MHz.
	
	Table Soccer has its sending of sample data interrupted by NMI, whose handler in turn wants to send its own sample data, causing audio hiccups in NTSC mode. PAL (Dendy) mode is okay.
*/

static const uint8_t indexStep     [4] ={ 0,  0,  3,  5 };
static const uint8_t indexTable   [26] ={ 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 20, 20, 20, 20 };
static const int8_t  stepTable [4][21] ={
		{  0,   1,   1,   1,   1,   1,   2,   2,   2,   3,   3,   4,   5,   5,   6,   7,   8,  10,  11,  13,  15 },
		{  1,   3,   3,   3,   4,   4,   6,   6,   7,   9,  10,  12,  15,  16,  19,  22,  25,  30,  34,  40,  46 },
		{  3,   5,   5,   6,   7,   8,  10,  11,  13,  16,  18,  21,  25,  28,  32,  38,  43,  51,  58,  68,  78 },
		{  4,   7,   7,   8,  10,  11,  14,  15,  18,  22,  25,  29,  35,  39,  45,  53,  60,  71,  81,  95, 109 }
};

void ADPCM3Bit::decodeSample (uint8_t code) {
	int16_t predictor =output +stepTable[code &3][index] *(code &4? -1: 1);
	output =predictor <-128? -128: predictor >127? 127: predictor;
	index =indexTable[index +indexStep[code &3]];
}

ADPCM3Bit::ADPCM3Bit(int32_t _chipClock, int32_t _hostClock): chipClock(_chipClock), hostClock(_hostClock) {
	lowPassFilter.setFc(chipClock /512.0 *0.425 /hostClock);
	reset();
}

void ADPCM3Bit::reset() {
	chipCount =0;
	count =0;
	period =512;
	inhibit =0;
	command =0xFF;
	latch =0x00;
	data =0x0;
	bytesLeft =0;
	clock =false;
	ready =true;
	index =0;
	output =0;
	playing =false;
	input.clear();
	while (!frames.empty()) frames.pop();
}

void ADPCM3Bit::run() {
	chipCount +=chipClock;
	while (chipCount >=hostClock) {
		chipCount -=hostClock;
		if (inhibit) inhibit--;
		if (playing && !(++count %period)) {
			if (frames.size()) {
				decodeSample(frames.front() &7);
				frames.front() >>=3;
			}
			if (count >=period *21) {
				count =0;
				if (frames.size()) frames.pop();
			}
		}
	}
}

bool ADPCM3Bit::getAck() {
	return clock && inhibit? true: !clock;
}

int MAPINT ADPCM3Bit::getAudio(int) {
	return lowPassFilter.process(output*256.0 +1e-15);
}

bool ADPCM3Bit::getReady() const {
	return ready;
}

void ADPCM3Bit::setClock(bool newClock) {
	if (!clock && newClock) // rising edge: receive MSB nibble
		latch =data <<4;
	else
	if (clock && !newClock && !inhibit) { // falling edge: receive LSB nibble
		latch |=data &0xF;
		if (command ==0x55 && latch ==0xAA)  // Special byte sequence for reset
			reset();
		else
		if (bytesLeft) { // Data byte for existing command? Store in buffer
			input.push_back(latch);
			bytesLeft--;
		} else {         // New command byte
			switch (latch) {
				case 0x03: // Set playback period
					bytesLeft =2;
					index =0;
					output =0;
					break;
				case 0x04: // Flush and fill frame buffer
					while (!frames.empty()) frames.pop();
					bytesLeft =96;
					break;
				case 0x06: // Request sending one or more ADPCM frames.
				           // A frame consists of 8 bytes, each containing 21 codes.
					   // The chip responds by raising READY if its frame buffer is not yet full.
					   // Client MUST continue to send frame data until READY goes low.
					if (frames.size() <12) {
						bytesLeft =8;
						ready = true;
					}
					break;
				case 0x07: // Stop immediately
					index =0;
					output =0;
					while (!frames.empty()) frames.pop();
					playing =false;
					break;
				case 0x55: // First byte of reset
					break;
				default:
					EMU->DbgOut(L"Bad Command: %02X", latch);
					break;
			}
			command =latch;
		}

		if (!bytesLeft) { // Process command once all expected bytes have been received.
			switch(command) {
				case 0x03: // Set playback period				
					period =input[0] | input[1] <<8;
					if (period ==0.0) period =1.0;
					lowPassFilter.setFc(chipClock /period *0.425 /hostClock);
					break;
				case 0x06: // Send ADPCM frame
					// Load all eight frame bytes into one 64-bit number.
					// It contains 21 3-bit codes plus one end-of-message marker.
					uint64_t frame =0;
					for (int i =0; i <8; i++) frame |= (long long) input[i] <<(i *8);
					
					frames.push(frame &0x8000000000000000? 0: frame);
					
					// Lower READY signal once 12 frames have been received, otherwise more frame data is expected.
					if (frames.size() >=12) {
						playing = true;
						ready =false;
						inhibit =16384;
					} else
						bytesLeft =8;
					break;
			}
			input.clear();
		}
	}
	clock =newClock;
}

void ADPCM3Bit::setData(uint8_t newData) {
	data =newData;
}

int MAPINT ADPCM3Bit::saveLoad (STATE_TYPE stateMode, int stateOffset, unsigned char *stateData) {
	SAVELOAD_LONG(stateMode, stateOffset, stateData, count);
	SAVELOAD_LONG(stateMode, stateOffset, stateData, period);
	SAVELOAD_LONG(stateMode, stateOffset, stateData, inhibit);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, command);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, latch);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, data);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, bytesLeft);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, clock);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, ready);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, playing);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, index);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, output);

	// Save/load std::vector
	if (stateMode ==STATE_LOAD) {
		uint32_t tempSize =0;
		SAVELOAD_LONG(stateMode, stateOffset, stateData, tempSize);
		if (tempSize >96) tempSize =96;
		input.resize(tempSize);
		for (int i =0; i <96; i++) {
			uint8_t tempData =0;
			SAVELOAD_BYTE(stateMode, stateOffset, stateData, tempData);
			if (tempSize >0) {
				input[i] =tempData;
				tempSize--;
			}

		}
	} else {
		uint32_t tempSize =input.size();
		SAVELOAD_LONG(stateMode, stateOffset, stateData, tempSize);
		for (int i =0; i <96; i++) {
			uint8_t tempData =0;
			if (tempSize >0) {
				tempData =input[i];
				tempSize--;
			}
			SAVELOAD_BYTE(stateMode, stateOffset, stateData, tempData);
		}
	}

	// Save/load std::queue
	if (stateMode ==STATE_LOAD) {
		while (!frames.empty()) frames.pop();
		uint32_t tempSize =0;
		SAVELOAD_LONG(stateMode, stateOffset, stateData, tempSize);
		for (int i =0; i <12; i++) {
			uint64_t tempData =0;
			SAVELOAD_UL64(stateMode, stateOffset, stateData, tempData);
			if (tempSize >0) {
				frames.push(tempData);
				tempSize--;
			}
		}
	} else {
		auto frames2 =frames;
		uint32_t tempSize =frames2.size();
		SAVELOAD_LONG(stateMode, stateOffset, stateData, tempSize);
		for (int i =0; i <12; i++) {
			uint64_t tempData =false;
			if (tempSize >0) {
				tempData =frames2.front();
				frames2.pop();
				tempSize--;
			}
			SAVELOAD_UL64(stateMode, stateOffset, stateData, tempData);
		}
	}
	return stateOffset;
}
