#include "stdafx.h"
#include "Nintendulator.h"
#include "MapperInterface.h"
#include "PS2DevicePort.h"

void PS2DevicePort::setHostClock(bool setting) {
	hostClock =setting;
}

void PS2DevicePort::setHostData(bool setting) {
	hostData =setting;	
}

void PS2DevicePort::setDeviceClock(bool setting) {
	deviceClock =setting;	
}

void PS2DevicePort::setDeviceData(bool setting) {
	deviceData =setting;	
}

bool PS2DevicePort::getClock() const {
	return hostClock && deviceClock;
}

bool PS2DevicePort::getData() const {
	return hostData && deviceData;
}

void PS2DevicePort::setNextState(int newState) {
	//if (state !=newState) EI.DbgOut(L"Next state: %d", newState);
	cycles =PS2_CLOCKS;
	state =newState;
}

void PS2DevicePort::checkForSendRequest() {
	setDeviceData(true); // Release DATA line
	oneBits =0;
	// If host is sending a Start Bit, start clocking and receiving from host
	if (!getData())
		setNextState(100);
	else			
	if (!dataToHost.empty()) {
		//EI.DbgOut(L"%d bytes remaining, starting to send", dataToHost.size());
		// If host is not sending a Start Bit, and we have something to send, then start clocking and sending to host
		latch =dataToHost.front();
		dataToHost.pop();
		setNextState(200);
	} else {
		//if (state !=0) EI.DbgOut(L"Next state: 0");
		state =0;
	}
}

void PS2DevicePort::cpuCycle() {
	if (cycles && deviceClock ==getClock()) {
		if (!--cycles) {
			setDeviceClock(!deviceClock);
			if (!deviceClock) cycles =PS2_CLOCKS;
		}
	} else
	if (deviceClock) {
		if (!getClock()) { // If CLOCK should be high but is low, then host is holding CLOCK low, which means Terminate Command and Inhibit Communication
			state =0;
			cycles =0;
			//EI.DbgOut(L"Clock held low");
		} else
		switch(state) {
			case 0: // Idle state, check if it needs to change
				checkForSendRequest();
				break;		
			case 100: // Receive the start bit (0) from host
				setNextState(getData()? 0: state +1);
				break;
			case 101: case 102: case 103: case 104: case 105: case 106: case 107: case 108: // Receive eight data bits from host
				latch =latch >>1 | getData()*0x80;
				if (getData()) oneBits++;
				setNextState(state +1);
				break;
			case 109: // Receive parity bit from host
				//EI.DbgOut(L"The data received was %02X", latch);
				if (getData() !=!!(~oneBits &1)) {
					//EI.DbgOut(L"Wrong parity bit");
					//dataToHost.push(0xFE); // Re-send
				}
				setNextState(state +1);
				break;
			case 110: // Receive stop bit from host
				if (getData()) {					
					setDeviceData(false); // Line-control bit
					dataFromHost.push(latch);
				} else {
					//EI.DbgOut(L"Wrong stop bit");
					setDeviceData(true);
					dataToHost.push(0xFE);
				}
				setNextState(state +1);
				break;
			case 111: // End
				setDeviceData(true); // Release data line to end transmission
				checkForSendRequest();
				break;
			case 200: // Send start bit to host
				setDeviceData(true); // Check if line held
				if (deviceData && !getData()) {
					//EI.DbgOut(L"Data reception interrupted by host wanting to send data");
					while (!dataToHost.empty()) dataToHost.pop();
					setNextState(101);
					setDeviceData(true); // Release data line
				} else {
					setNextState(state +1);
					setDeviceData(false); // Start bit
				}
				break;
			case 201: case 202: case 203: case 204: case 205: case 206: case 207: case 208: // Send eight data bits to host
				setDeviceData(true); // Check if line held
				if (deviceData && !getData()) {
					//EI.DbgOut(L"Data reception interrupted by host wanting to send data");
					while (!dataToHost.empty()) dataToHost.pop();
					setNextState(101);
					setDeviceData(true); // Releae data line
				} else {
					setNextState(state +1);
					setDeviceData(latch &0x01);
					if (latch &0x01) oneBits++;
					latch =latch >>1;
				}
				break;
			case 209: // Send parity bit to host
				setDeviceData(~oneBits &1);
				setNextState(state +1);
				break;
			case 210: // Send stop bit to host
				setDeviceData(true); // Stop bit
				setNextState(state +1);
				break;
			case 211: // End
				checkForSendRequest();
				break;
		}
	}
}

PS2DevicePort::PS2DevicePort() {
	reset();
}

void PS2DevicePort::reset() {
	setHostClock(true);
	setHostData(true);
	setDeviceClock(true);
	setDeviceData(true);
	state = 0;
	cycles = 0;
	latch = 0;
	oneBits = 0;
	while (!dataFromHost.empty()) dataFromHost.pop();
	while (!dataToHost.empty()) dataToHost.pop();
}

int PS2DevicePort::saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	uint8_t temp = 0, len = 0;
	if (stateMode ==STATE_LOAD) { // Unpack bits
		SAVELOAD_BYTE(stateMode, offset, data, temp);
		hostClock   = !!(temp &0x01);
		hostData    = !!(temp &0x02);
		deviceClock = !!(temp &0x04);
		deviceData  = !!(temp &0x08);
	} else { // Pack bits
		temp = hostClock*0x01 + hostData*0x02 + deviceClock*0x04 + deviceData*0x08;
		SAVELOAD_BYTE(stateMode, offset, data, temp);
	}
	SAVELOAD_WORD(stateMode, offset, data, state);
	SAVELOAD_LONG(stateMode, offset, data, cycles);	
	SAVELOAD_BYTE(stateMode, offset, data, latch);
	SAVELOAD_BYTE(stateMode, offset, data, oneBits);
	
	if (stateMode ==STATE_LOAD) { // Deserialize containers
		while (!dataFromHost.empty()) dataFromHost.pop();
		SAVELOAD_BYTE(stateMode, offset, data, len); // # of bytes that follow
		while (len--) {
			SAVELOAD_BYTE(stateMode, offset, data, temp);
			dataFromHost.push(temp);
		}

		while (!dataToHost.empty()) dataToHost.pop();
		SAVELOAD_BYTE(stateMode, offset, data, len); // # of bytes that follow
		while (len--) {
			SAVELOAD_BYTE(stateMode, offset, data, temp);
			dataToHost.push(temp);
		}
	} else { // Serialize containers
		std::queue<uint8_t> queue;
		
		queue = dataFromHost;
		len = queue.size() >255? 255: 0;
		SAVELOAD_BYTE(stateMode, offset, data, len);
		while (len--) {
			SAVELOAD_BYTE(stateMode, offset, data, queue.front());
			queue.pop();
		}
	
		queue = dataToHost;
		len = queue.size() >255? 255: 0;
		SAVELOAD_BYTE(stateMode, offset, data, len);
		while (len--) {
			SAVELOAD_BYTE(stateMode, offset, data, queue.front());
			queue.pop();
		}
	}	
	return offset;
}
