#define PS2_CLOCKS 70 // Too low, and BBKDOS' mouse driver will fail.
#include <queue>

class PS2DevicePort {
	bool hostClock;
	bool hostData;
	bool deviceClock;
	bool deviceData;
	uint16_t state;
	int32_t  cycles;
	uint8_t latch;
	uint8_t oneBits;
	void setDeviceClock(bool);
	void setDeviceData(bool);
	void setNextState(int);
	void checkForSendRequest();
public:
	std::queue<uint8_t> dataFromHost;
	std::queue<uint8_t> dataToHost;
	void setHostClock(bool);
	void setHostData(bool);
	bool getClock() const;
	bool getData() const;

	void reset();
	void cpuCycle();
	int saveLoad (STATE_TYPE, int, unsigned char *);
	PS2DevicePort();
};

