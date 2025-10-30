#include "h_FIFO.hpp"

size_t FIFO::size (void) {
	front %= capacity;
	back %= capacity;
	return (back -front +capacity) %capacity;
}
bool FIFO::halfFull (void) {
	return size() >=capacity /2;
}

int FIFO::retrieve (void) {
	front %= capacity;
	return size()? data[front++]: 0;
}

void FIFO::add (uint8_t value) {
	back %= capacity;
	if (size() <capacity) data[back++] = value;
}

void FIFO::reset (void) {
	front = back = 0;
}

FIFO::FIFO (size_t newCapacity): capacity(newCapacity) {
	data = new uint8_t[newCapacity];
}

FIFO::~FIFO () {
	delete[] data;
}

int MAPINT FIFO::saveLoad (STATE_TYPE stateMode, int offset, unsigned char *stateData) {
	for (unsigned int i = 0; i < capacity; i++) SAVELOAD_BYTE(stateMode, offset, stateData, data[i]);
	SAVELOAD_WORD(stateMode, offset, stateData, front);
	SAVELOAD_WORD(stateMode, offset, stateData, back);
	return offset;
}

