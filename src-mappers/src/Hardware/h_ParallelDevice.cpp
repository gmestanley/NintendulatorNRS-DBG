#include	"h_ParallelDevice.h"

// Generalized parallel device
ParallelDevice::ParallelDevice():
	state(0),
	clockToDevice(true),
	clockFromDevice(true),
	dataToDevice(0xFF),
	dataFromDevice(0xFF) {
}

void ParallelDevice::reset() {
	state =0;
	clockToDevice =true;
	clockFromDevice =true;
	dataToDevice =0xFF;
	dataFromDevice =0xFF;
}

bool ParallelDevice::getClock() {
	return clockFromDevice;
}

uint8_t ParallelDevice::getData() const {
	return dataFromDevice;
}

void ParallelDevice::setPins(bool select, bool newClock, uint8_t newData) {
	if (!select) {
		clockToDevice =newClock;
		dataToDevice =newData;
	}
}

int MAPINT ParallelDevice::saveLoad (STATE_TYPE stateMode, int stateOffset, unsigned char *stateData) {
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, state);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, clockToDevice);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, clockFromDevice);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, dataToDevice);
	SAVELOAD_BYTE(stateMode, stateOffset, stateData, dataFromDevice);
	return stateOffset;
}
