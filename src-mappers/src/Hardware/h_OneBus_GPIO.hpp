#pragma once
#include	"..\interface.h"
#include 	"h_SerialDevice.h"
#include 	"h_ParallelDevice.h"

namespace OneBus {
class GPIO {
	struct AttachedSerialDevice {
		SerialDevice* device;
		uint8_t       select;
		uint8_t       clock;
		uint8_t       data;
	};
	struct AttachedParallelDevice {
		ParallelDevice* device;
		uint8_t       select;
		uint8_t       clockToDevice;
		uint8_t       clockFromDevice;
		uint8_t       dataToDevice;
		uint8_t       dataToDeviceMask;
		uint8_t       dataFromDevice;
		uint8_t       dataFromDeviceMask;
	};
	uint8_t mask; // clear bit=read, set bit=write
	uint8_t latch;
	uint8_t state;
	std::vector<AttachedSerialDevice> serialDevices;
	std::vector<AttachedParallelDevice> parallelDevices;
public:
	        GPIO();
	uint8_t read(uint8_t);
	void    write(uint8_t, uint8_t);
	void    attachSerialDevice(AttachedSerialDevice);
	void    attachParallelDevice(AttachedParallelDevice);
	void    reset();
};

} // namespace OneBus