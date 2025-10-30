#include "h_latch.h"

namespace Latch {
bool lockable;
uint8_t data;
uint8_t dataLocked;
uint16_t addr;
uint16_t addrLocked;
uint16_t addrMaskAND;
uint16_t addrMaskVal;
FSync sync;
FBusConflict busConflict = NULL;

void MAPINT write (int newBank, int newAddr, int newData) {
	*EMU->OpenBus = newData; // If there is open bus at the write address, it should not cause a conflict with the value being written
	newData = busConflict? busConflict(newData, EMU->GetCPUReadHandler(newBank)(newBank, newAddr)): newData;
	newAddr = newBank <<12 |newAddr;
	if (lockable) {
		newAddr = addr &addrLocked | newAddr &~addrLocked;
		newData = data &dataLocked | newData &~dataLocked;
	}
	addr = newAddr;
	data = newData;
	sync();
}

void MAPINT setLockedBits (uint16_t newAddrLocked, uint8_t newDataLocked) {
	addrLocked = newAddrLocked;
	dataLocked = newDataLocked;
}

int busConflictAND (int cpuData, int romData) {
	return cpuData &romData;
}

int busConflictOR (int cpuData, int romData) {
	return cpuData |romData;
}

int busConflictROM (int cpuData, int romData) {
	return romData;
}

void MAPINT load (FSync _sync, FBusConflict _busConflict, bool _lockable) {
	sync = _sync;
	busConflict = _busConflict;
	lockable = _lockable;
}

void MAPINT load (FSync _sync, FBusConflict _busConflict) {
	load(_sync, _busConflict, false);
}

void MAPINT load (FSync _sync) {
	load(_sync, NULL, false);
}

void MAPINT reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		data = 0;
		addr = 0;
		dataLocked = 0;
		addrLocked = 0;
	}
	sync();
	for (int bank = 0x8; bank <= 0xF; bank++) EMU->SetCPUWriteHandler(bank, write);
}

void MAPINT resetHard (RESET_TYPE resetType) {
	reset(RESET_HARD);
}

int MAPINT saveLoad_AD (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_WORD(stateMode, offset, _data, addr);
	SAVELOAD_BYTE(stateMode, offset, _data, data);
	if (lockable) {
		SAVELOAD_WORD(stateMode, offset, _data, addrLocked);
		SAVELOAD_BYTE(stateMode, offset, _data, dataLocked);
	}
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int MAPINT saveLoad_AL (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_BYTE(stateMode, offset, _data, addr);
	if (lockable) SAVELOAD_BYTE(stateMode, offset, _data, addrLocked);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int MAPINT saveLoad_A (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_WORD(stateMode, offset, _data, addr);
	if (lockable) SAVELOAD_WORD(stateMode, offset, _data, addrLocked);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int MAPINT saveLoad_D (STATE_TYPE stateMode, int offset, unsigned char *_data) {
	SAVELOAD_BYTE(stateMode, offset, _data, data);
	if (lockable) SAVELOAD_BYTE(stateMode, offset, _data, dataLocked);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

} // namespace Latch