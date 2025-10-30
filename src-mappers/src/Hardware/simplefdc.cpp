#include "../interface.h"
#include <stdio.h>
#include <stdint.h>
#include <cstdarg>
#include <vector>
#include <queue>
#include "simplefdc.hpp"

static constexpr uint8_t
	DOR_SELECT_DRIVE =0x03,
	DOR_NOT_RESET =0x04, // "Software" reset
	DOR_MOTOR_A =0x10,
	DOR_MOTOR_B =0x20,
	DOR_MOTOR_C =0x40,
	DOR_MOTOR_D =0x80,
	MSR_READY =0x80,
	MSR_BUSY =0x10,
	MSR_DIRECTION =0x40,
	MSR_NO_DMA =0x20,
	CMD_MULTI_TRACK =0x80,
	CMD_NUMBER =0,
	CMD_US_HD =1,
	CMD_CYLINDER =2,
	CMD_HEAD =3,
	CMD_SECTOR_NUMBER =4,
	CMD_SECTOR_SIZE =5,
	CMD_LAST_SECTOR =6,
	CMD_FORMAT_SECTOR_SIZE =2,
	CMD_FORMAT_LAST_SECTOR =3,
	CMD_FORMAT_FILL =5,
	CMD_NCN =2,
	ST0_NORMAL_TERMINATION =0x00,
	ST0_ABNORMAL_TERMINATION =0x40,
	ST0_INVALID_COMMAND =0x80,
	ST0_POLLING =0xC0,
	ST0_SEEK_COMPLETE =0x20,
	ST0_NOT_READY =0x08,
	ST1_END_OF_TRACK =0x80,
	ST1_OVERRUN =0x10,
	ST1_NO_DATA =0x04,
	ST1_NOT_WRITEABLE =0x02,
	ST1_MISSING_ADDRESS_MARK =0x01,
	ST2_WRONG_CYLINDER =0x10,
	SRA_IRQ =0x80,
	CFG_NOT_POLL =0x10
;

const CommandInfo FDC::commandInfo[0x20] ={
	{ 1, &FDC::cmInvalid }, 		// 00
	{ 1, &FDC::cmInvalid }, 		// 01
	{ 9, &FDC::cmInvalid }, 		// 02
	{ 3, &FDC::cmSpecify }, 		// 03
	{ 2, &FDC::cmSenseDriveStatus },	// 04
	{ 9, &FDC::cmWriteData },		// 05
	{ 9, &FDC::cmReadData },		// 06
	{ 2, &FDC::cmRecalibrate },		// 07
	{ 1, &FDC::cmSenseInterruptStatus },	// 08
	{ 9, &FDC::cmInvalid },			// 09
	{ 2, &FDC::cmReadID },			// 0A
	{ 1, &FDC::cmInvalid },			// 0B
	{ 9, &FDC::cmInvalid },			// 0C
	{ 6, &FDC::cmFormatTrack },		// 0D
	{ 1, &FDC::cmInvalid },			// 0E
	{ 3, &FDC::cmSeek },			// 0F
	{ 1, &FDC::cmInvalid },			// 10
	{ 9, &FDC::cmInvalid },			// 11
	{ 2, &FDC::cmInvalid },			// 12
	{ 4, &FDC::cmConfigure },		// 13
	{ 1, &FDC::cmInvalid },			// 14
	{ 1, &FDC::cmInvalid },			// 15
	{ 9, &FDC::cmInvalid },			// 16
	{ 9, &FDC::cmInvalid },			// 19
	{ 9, &FDC::cmInvalid },			// 1D
	{ 1, &FDC::cmInvalid },			// 1E
	{ 1, &FDC::cmInvalid }			// 1F
};
static const int dataRates[4] = { 500, 300, 250, 1000 };

FDC::FDC() {
	resetPin =false;
	DOR =CONFIG =pollTimer =pollDrive =0;
	dataRate =dataRates[0];
	for (int i =0; i <4; i++) {
		previouslyReady[i] =false;
		Drive *d =&drives[i];
		d->exists =true;
		d->ready =true;
		d->inserted =false;
		d->running =false;
		d->cylinder =0;
		d->cylinders =0;
		d->sides =0;
		d->sectors =0;
		d->correctDataRate =0;
		d->data =NULL;
	}
	useDMA =false; // not affected by Reset
	drive =&drives[0];
	reset();
}

void	FDC::run (void) {
	if (DOR &DOR_NOT_RESET && !resetPin) (this->*phaseHandler)();
}

void	FDC::reset () {
	phaseHandler =&FDC::phCommand;
	MSR =MSR_READY;
	SRA =ST0 =ST1 =ST2 =0x00;
	/*cylinder =head =sectorNumber =sectorSize =*/bytesLeft =0;	// Do not reset disk position because Subor V resets the FDC between sectors
	data =NULL;
	command.clear();
	while (!resultQueue.empty()) resultQueue.pop();
	dmaRequest =dmaTerminate =false;
	
	pollTimer =pollDrive =0;
	for (int i =0; i <4; i++) previouslyReady[i] =false;
}

void	FDC::phCommand (void) {
	pollDrives();
	if (~MSR &MSR_READY) {	// Data byte has been written to the latch
		MSR |=MSR_BUSY;
		command.push_back(latch);

		// If all required bytes of the desired command have been sent,
		// start the Execution Phase, otherwise wait for more bytes
		if (command.size() ==commandInfo[command[CMD_NUMBER] &0x1F].length) {
			char commandString[1024] ="Cmd: ";
			for (auto& c: command) {
				char single[32];
				snprintf(single, 31, "%02X ", c);
				strncat_s(commandString, single, 1023);
			}
			//EMU->DbgOut(L"%S", commandString);

			phaseHandler =commandInfo[command[CMD_NUMBER] &0x1F].handler;
			data =NULL;
			bytesLeft =0;
			if ((command[CMD_NUMBER] &0x1F) !=0x08) {
				ST0 =command[CMD_US_HD] &7;
				ST1 =ST2 =0x00;
			}
		} else
			MSR |=MSR_READY;
	}
}

void	FDC::phResult (void) {
	if (~MSR &MSR_READY) {
		// If all result bytes have been read, start the Command Phase
		// otherwise provide more bytes
		if (resultQueue.empty()) {
			MSR &=~MSR_DIRECTION;
			MSR &=~MSR_BUSY;
			command.clear();
			phaseHandler =&FDC::phCommand;
			dmaRequest =false;
		} else {
			MSR |= MSR_DIRECTION;
			latch =resultQueue.front();
			resultQueue.pop();
		}
		MSR |=MSR_READY;
	}
}

void	FDC::endCommand (bool raiseIRQ, int num, ...) {
	va_list list;
	va_start(list, num);

	char resultString[1024] ="Result: ";
	for (int i =0; i <num; i++) {
		int value =va_arg(list, int);
		resultQueue.push(value);
		char single[32];
		snprintf(single, 31, "%02X ", value);
		strncat_s(resultString, single, 1023);
	}
	//if (num) EMU->DbgOut(L"%S", resultString);
	va_end(list);

	phaseHandler =&FDC::phResult;
	if (raiseIRQ) SRA |=SRA_IRQ;
	MSR &=~MSR_NO_DMA;
}

void	FDC::nextSector (void) {
	if (data ==NULL) {
		cylinder =command[CMD_CYLINDER];
		head =command[CMD_HEAD];
		sectorNumber =command[CMD_SECTOR_NUMBER];
		sectorSize =128 <<command[CMD_SECTOR_SIZE];
		if (sectorSize >8192) sectorSize =8192;
	} else
	if (sectorNumber ==command[CMD_LAST_SECTOR]) {
		if (command[CMD_NUMBER] &CMD_MULTI_TRACK && head ==0) {
			head ^=1;
			command[CMD_HEAD] ^=1;
			sectorNumber =1;
		} else
		if (useDMA && dmaTerminate) {
			cylinder++;
			head ^=1;
			sectorNumber =1;
		} else {
			ST0 |=ST0_ABNORMAL_TERMINATION;
			ST1 |=ST1_END_OF_TRACK;
			endCommand(true, 7, ST0, ST1, ST2, cylinder, head, sectorNumber, command[CMD_SECTOR_SIZE]);
			return;
		}
	} else
		sectorNumber++;

	if (useDMA && dmaTerminate /* || SIGNAL_SH */) {
		ST0 |=ST0_NORMAL_TERMINATION;
		endCommand(true, 7, ST0, ST1, ST2, cylinder, head, sectorNumber, command[CMD_SECTOR_SIZE]);
		dmaTerminate =false;
		return;
	}

	if (!drive->inserted || cylinder >=drive->cylinders || head >=drive->sides || !drive->running || dataRate !=drive->correctDataRate) {
		EMU->DbgOut(L"Disk transfer error on drive %d: %S", CMD_US_HD, !drive->inserted? "no disk in drive": cylinder >=drive->cylinders? "cylinder": head >=drive->sides? "head": !drive->running? "not running": dataRate !=drive->correctDataRate? "rate": "unknown");
		ST1 |=ST1_MISSING_ADDRESS_MARK;
		ST0 |=ST0_ABNORMAL_TERMINATION;
	} else
	if (cylinder !=drive->cylinder) {
		EMU->DbgOut(L"FDC: wrong cylinder");
		ST1 |=ST1_NO_DATA;
		ST2 |=ST2_WRONG_CYLINDER;
		ST0 |=ST0_ABNORMAL_TERMINATION;
	} else
	if (sectorNumber <1 || sectorNumber >drive->sectors || sectorSize !=512) {
		//EMU->DbgOut(L"FDC: sector %d-%d-%d (%d) not found", cylinder, head, sectorNumber, sectorSize);
		ST1 |=ST1_NO_DATA;
		ST0 |=ST0_ABNORMAL_TERMINATION;
	}

	if (ST0 &ST0_ABNORMAL_TERMINATION) {
		endCommand(true, 7, ST0, ST1, ST2, cylinder, head, sectorNumber, command[CMD_SECTOR_SIZE]);
		return;
	} else {
		int lba =(cylinder *drive->sides + head) *drive->sectors +sectorNumber -1;
		bytesLeft =sectorSize;
		data =drive->data +lba *sectorSize;
	}
}

void	FDC::cmReadID (void) {
	if (!drive->inserted || cylinder >=drive->cylinders || head >=drive->sides || !drive->running || dataRate !=drive->correctDataRate) {
		//EMU->DbgOut(L"Read ID error on drive %d: %S", CMD_US_HD, !drive->inserted? "no disk in drive": cylinder >=drive->cylinders? "cylinder": head >=drive->sides? "head": !drive->running? "not running": dataRate !=drive->correctDataRate? "rate": "unknown");
		ST1 |=ST1_MISSING_ADDRESS_MARK;
		ST0 |=ST0_ABNORMAL_TERMINATION;
	} else {
		cylinder =drive->cylinder;
		head =command[CMD_US_HD] >>2;;
		sectorSize =512;
		if (sectorNumber ==0 || sectorNumber >=drive->sectors) sectorNumber =1;
		ST0 |=ST0_NORMAL_TERMINATION;
	}
	endCommand(true, 7, ST0, ST1, ST2, cylinder, head, sectorNumber, command[CMD_SECTOR_SIZE]);
}

void	FDC::cmReadData (void) {
	if (~MSR &MSR_READY && !dmaRequest) { // Do nothing if we're waiting for CPU to collect a byte (don't emulate overrun)
		if (bytesLeft ==0) nextSector();
		if (bytesLeft) {
			latch =*data++;
			bytesLeft--;
			if (useDMA)
				dmaRequest =true;
			else {
				MSR |=MSR_READY;
				MSR |=MSR_NO_DMA;
				MSR |=MSR_DIRECTION;
				SRA |=SRA_IRQ;
			}
		}
	}
}

void	FDC::cmWriteData (void) {
	if (drive->writeProtected) {
		ST0 |=ST0_ABNORMAL_TERMINATION;
		ST1 |=ST1_NOT_WRITEABLE;
		endCommand(true, 7, ST0, ST1, ST2, cylinder, head, sectorNumber, command[CMD_FORMAT_SECTOR_SIZE]);
		return;
	}
	if (~MSR &MSR_READY && !dmaRequest) { // Do nothing if we're waiting for CPU to provide a byte (don't emulate overrun)
		if (bytesLeft ==0)
			nextSector();
		else {
			*data++ =latch;
			bytesLeft--;
			if (drive->modifiedFlag) *drive->modifiedFlag =true;
		}
		if (bytesLeft) {
			if (useDMA)
				dmaRequest =true;
			else {
				MSR |= MSR_READY;
				MSR |= MSR_NO_DMA;
				MSR &=~MSR_DIRECTION;
				SRA |= SRA_IRQ;
			}
		}
	}
}

void	FDC::cmFormatTrack (void) {
	if (drive->writeProtected) {
		ST0 |=ST0_ABNORMAL_TERMINATION;
		ST1 |=ST1_NOT_WRITEABLE;
		endCommand(true, 7, ST0, ST1, ST2, cylinder, head, sectorNumber, command[CMD_FORMAT_SECTOR_SIZE]);
		return;
	}
	if (~MSR &MSR_READY) {	// Do nothing if we're waiting for CPU to provide a byte (don't emulate overrun)
		if (bytesLeft ==0) {
			if (data ==NULL) {
				cylinder =drive->cylinder;
				head =command[CMD_US_HD] >>2;
				sectorNumber =1;
			} else
			if (++sectorNumber >command[CMD_FORMAT_LAST_SECTOR]) {
				ST0 |=ST0_NORMAL_TERMINATION;
				endCommand(true, 7, ST0, ST1, ST2, cylinder, head, sectorNumber, command[CMD_FORMAT_SECTOR_SIZE]);
				return;
			}
			int lba =(cylinder *drive->sides + head) *drive->sectors +sectorNumber -1;
			bytesLeft =sectorSize;
			data =drive->data +lba *sectorSize;
		} else {
			*data++ =command[CMD_FORMAT_FILL];
			bytesLeft--;
			if (drive->modifiedFlag) *drive->modifiedFlag =true;
		}
		if (bytesLeft > 512-4) { // request the sector ID while we are filling the first four bytes of the sector
			MSR |= MSR_READY;
			MSR |= MSR_NO_DMA;
			MSR &=~MSR_DIRECTION;
			SRA |= SRA_IRQ;
		}
	}
}

void	FDC::cmSeek (void) {
	if (drive->cylinder !=command[CMD_NCN]) drive->changed =false;
	drive->cylinder =command[CMD_NCN];
	ST0 |=ST0_SEEK_COMPLETE;
	if (!drive->exists) ST0 |=ST0_ABNORMAL_TERMINATION;
	endCommand(true, 0);
}

void	FDC::cmRecalibrate (void) {
	if (drive->cylinder !=0) drive->changed =false;
	drive->cylinder =0;
	ST0 |=ST0_SEEK_COMPLETE;
	if (!drive->exists) ST0 |=ST0_ABNORMAL_TERMINATION;
	endCommand(true, 0);
}

void	FDC::cmSenseInterruptStatus (void) {
	SRA &=~SRA_IRQ;
	endCommand(false, 2, ST0, drive->cylinder);
}

void	FDC::pollDrives (void) {
	if (CONFIG &CFG_NOT_POLL) return;
	if (!pollTimer) {
		pollTimer =394; // polling duration 220 µs
		if (previouslyReady[pollDrive] !=drives[pollDrive].ready) {
			previouslyReady[pollDrive] =drives[pollDrive].ready;
			ST0 =ST0 &~ST0_SEEK_COMPLETE | ST0_POLLING | pollDrive;
			SRA |=SRA_IRQ;
		} 
		pollDrive =(pollDrive +1) &3;
	} else
		pollTimer--;
}

void	FDC::cmSpecify (void) {
	useDMA =!(command [2] &1);
	endCommand(false, 0);
}

void	FDC::cmSenseDriveStatus (void) {
	endCommand(false, 1, (DOR &DOR_SELECT_DRIVE) |
	                     (head <<2) |
			     ((drive->cylinder ==0)? 0x10: 0x00) |
			     (drive->writeProtected? 0x40: 0x00) |
			     (drive->inserted? 0x20: 0x00) |
			     0x28 );
}

void	FDC::cmConfigure (void) {
	EMU->DbgOut(L"FDC: Configure %02X", command[2]);
	CONFIG =command[2];
	endCommand(false, 0);
}

void	FDC::cmInvalid (void) {
	EMU->DbgOut(L"FDC: Invalid command %02X", command[CMD_NUMBER]);
	ST0 |=ST0_INVALID_COMMAND;
	endCommand(false, 1, ST0);
}

int	FDC::readIO (int Addr) {
	int	result =0xFF;
	switch (Addr &7) {
		case 0:	result =SRA;
			break;
		case 2:	result =DOR;
			break;
		case 3: result =SRA &SRA_IRQ? 0x40: 0;
			break;
		case 4:	result =MSR;
			break;
		case 5:	if (MSR &MSR_READY && MSR &MSR_DIRECTION) {
				MSR &=~MSR_READY;
				SRA &=~SRA_IRQ;
				result =latch;
			}
			break;
		case 7:	result =drive->changed? 0x80: 0x00;
			drive->changed =false;
			break;
		default:EMU->DbgOut(L"Read from unmapped FDC address %X", Addr &7);
			break;
	}
	return result;
}

int	FDC::readIODebug (int Addr) {
	int	result =0xFF;
	switch (Addr &7) {
		case 0:	result =SRA;
			break;
		case 2:	result =DOR;
			break;
		case 4:	result =MSR;
			break;
		case 5:	if (MSR &MSR_READY && MSR &MSR_DIRECTION) result =latch;
			break;
	}
	return result;
}

void	FDC::writeIO (int Addr, int Val) {
	switch (Addr &7) {
		case 2:	if (DOR &DOR_NOT_RESET && ~Val &DOR_NOT_RESET) reset();
			DOR =Val;
			drive =&drives[DOR &DOR_SELECT_DRIVE];
			drives[0].running =!!(DOR &DOR_MOTOR_A && drives[0].exists);
			drives[1].running =!!(DOR &DOR_MOTOR_B && drives[1].exists);
			drives[2].running =!!(DOR &DOR_MOTOR_C && drives[2].exists);
			drives[3].running =!!(DOR &DOR_MOTOR_D && drives[3].exists);
			break;
		case 3:	SRA &=~SRA_IRQ;
			break;
		case 5:	if (MSR &MSR_READY && ~MSR &MSR_DIRECTION) {
				MSR &=~MSR_READY;
				SRA &=~SRA_IRQ;
				latch =Val;
			}
			break;
		case 7:	dataRate =dataRates[Val &3];
			break;
		default:EMU->DbgOut(L"Write %02X to unmapped FDC address %X", Val, Addr &7);
			break;
	}
}

void	FDC::insertDisk (uint8_t driveNum, uint8_t *_data, size_t _size, bool _writeProtected, bool *modifiedFlag) {
	Drive *d =&drives[driveNum &3];
	d->data =_data;
	d->writeProtected =_writeProtected;
	d->modifiedFlag =modifiedFlag;
	switch (_size) {
		case 160*1024:
			d->cylinders =40;
			d->sides =1;
			d->sectors =8;
			d->correctDataRate =250;
			break;
		case 180*1024:
			d->cylinders =40;
			d->sides =1;
			d->sectors =9;
			d->correctDataRate =250;
			break;
		case 320*1024:
			d->cylinders =40;
			d->sides =2;
			d->sectors =8;
			d->correctDataRate =250;
			break;
		case 360*1024:
			d->cylinders =40;
			d->sides =2;
			d->sectors =9;
			d->correctDataRate =250;
			break;
		case 720*1024:
			d->cylinders =80;
			d->sides =2;
			d->sectors =9;
			d->correctDataRate =250;
			break;
		case 1200*1024:
			d->cylinders =80;
			d->sides =2;
			d->sectors =15;
			d->correctDataRate =500;
			break;
		case 1440*1024:
			d->cylinders =80;
			d->sides =2;
			d->sectors =18;
			d->correctDataRate =500;
			break;
		case 2880*1024:
			d->cylinders =80;
			d->sides =2;
			d->sectors =36;
			d->correctDataRate =1000;
			break;
		default:
			EMU->DbgOut(L"Unknown disk size: %u", _size);
			break;
	}
	d->inserted =true;
}

void	FDC::ejectDisk (uint8_t driveNum) {
	Drive *d =&drives[driveNum &3];
	d->data =NULL;
	if (d->inserted) d->changed =true;
	d->inserted =false;
}

bool	FDC::irqRaised (void) const {
	return !!(SRA &SRA_IRQ);
}

bool	FDC::haveDMARequest (void) const {
	return dmaRequest;
}

void	FDC::setTerminalCount (void) {
	dmaTerminate =true;
}

void	FDC::sendToDMA (uint8_t val) {
	latch =val;
	dmaRequest =false;
}

uint8_t	FDC::receiveFromDMA (void) {
	dmaRequest =false;
	return latch;
}

void	FDC::setDriveMask (uint8_t mask) {
	drive[0].exists =drive[0].ready =!!(mask &1);
	drive[1].exists =drive[1].ready =!!(mask &2);
	drive[2].exists =drive[2].ready =!!(mask &4);
	drive[3].exists =drive[3].ready =!!(mask &8);
}

void	FDC::setResetPin (bool pin) {
	if (!resetPin && pin) reset();
	resetPin =pin;
}
