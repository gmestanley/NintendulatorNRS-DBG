#include "h_OneBus_GPIO.hpp"

namespace OneBus {

GPIO::GPIO():
	mask(0),
	latch(0),
	state(0xFF) {
}
uint8_t GPIO::read(uint8_t address) {
	switch(address &7) {
		case 0:	return mask;
		case 2: return latch;
		case 3: state =~mask;
			for (auto& d: serialDevices) state &=d.device->getData() <<d.data | ~(1 <<d.data);
			for (auto& d: parallelDevices) state &=d.device->getData()  <<d.dataFromDevice | ~(d.dataFromDeviceMask <<d.dataFromDevice);
			for (auto& d: parallelDevices) state &=d.device->getClock() <<d.clockFromDevice | ~(1 <<d.clockFromDevice);
			return state;
		default:return 0xFF;
	}
}
void    GPIO::write(uint8_t address, uint8_t value) {
	switch(address &7) {
		case 0:	mask =value;
			state =state &~mask | latch &mask;
			for (auto& d: serialDevices) d.device->setPins(state >>d.select &1, state >>d.clock &1, state >>d.data &1);
			for (auto& d: parallelDevices) d.device->setPins(state >>d.select &1, state >>d.clockToDevice &1, state >>d.dataToDevice &d.dataToDeviceMask);
			break;
		case 2:
		case 3:	latch =value;
			state =state &~mask | latch &mask;
			for (auto& d: serialDevices) d.device->setPins(state >>d.select &1, state >>d.clock &1, state >>d.data &1);
			for (auto& d: parallelDevices) d.device->setPins(state >>d.select &1, state >>d.clockToDevice &1, state >>d.dataToDevice &d.dataToDeviceMask);
			break;
	}
}
void    GPIO::attachSerialDevice(AttachedSerialDevice device) {
	serialDevices.push_back(device);
}

void    GPIO::attachParallelDevice(AttachedParallelDevice device) {
	parallelDevices.push_back(device);
}

void    GPIO::reset() {
	mask =0;
	state =0xFF;
	serialDevices.clear();
	parallelDevices.clear();
}

} // namespace OneBus