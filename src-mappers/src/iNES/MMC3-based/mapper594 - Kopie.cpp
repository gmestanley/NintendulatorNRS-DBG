#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_MMC3.h"
#include	"..\..\Hardware\Sound\Butterworth.h"

namespace {
uint8_t reg[4];

class FIFO {
	uint8_t data[1024];
	int16_t front;
	int16_t back;
public: 
	size_t size (void) {
		front &= 1023;
		back &= 1023;
		return (back -front +1024) &1023;
	}
	bool halfFull (void) {
		return size() >=512;
	}
	uint8_t retrieve (void) {
		front &= 1023;
		return size()? data[front++]: 0;
	}
	void add (uint8_t value) {
		back &= 1023;
		if (size() <1024) data[back++] = value;
	}
	void reset (void) {
		front = back = 0;
	}
} fifo;
bool adpcmNibble;
uint8_t adpcmByte;
uint32_t adpcmCount;
int32_t adpcmOutput;
int32_t signal;
int16_t step;
int32_t rate = 4000;
//LPF_RC lowPassFilter(4000 *0.425 /1789773.0);
Butterworth	lowPassFilter(10, 1789772.727272, 1600.0); 

void sync (void) {
	int prgAND = 0x3F;
	int chrAND = reg[2] &0xC0? 0x0FF: 0x1FF;
	int prgOR = (reg[2] &0x40? 0x0C0: 0x000) | (reg[2] &0x80? 0x100: 0x000);
	int chrOR = (reg[2] &0x40? 0x200: 0x000) | (reg[2] &0x80? 0x300: 0x000);
	EMU->SetPRG_ROM8(0x6, reg[0] | prgOR);
	MMC3::syncPRG(prgAND, prgOR &~prgAND);
	MMC3::syncCHR(chrAND, chrOR &~chrAND);
	MMC3::syncMirror();
}

int getCHRBank (int bank) {
	return MMC3::getCHRBank(bank) | bank <<6 &0x100;
}

int MAPINT readFIFO (int bank, int addr) {
	return fifo.halfFull()? 0x00: 0x40;
}

void MAPINT writeFIFO (int bank, int addr, int val) {
	if (addr &1) {
		if (val &0x40) {
			rate = 8000;
			lowPassFilter.recalc(4, 1789773.0, 4000);
		} else {
			rate = 4000;
			lowPassFilter.recalc(6, 1789773.0, 1600);
		}
		//lowPassFilter.setFc(rate *0.425 / 1789773.0);
		fifo.reset();
		signal = -2;
		step = 0;
	} else
		fifo.add(val);
}

void MAPINT writeReg (int bank, int addr, int val) {
	reg[bank &2 | addr &1] = val;
	sync();
}

BOOL MAPINT load (void) {
	iNES_SetSRAM();
	MMC3::load(sync, MMC3Type::AX5202P, NULL, getCHRBank, NULL, NULL);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg[0] = reg[1] = reg[2] = reg[3] = 0;
	adpcmCount = 0;
	adpcmNibble = false;
	signal = -2;
	step = 0;
	fifo.reset();
	MMC3::reset(resetType);	
	EMU->SetCPUReadHandler(0x5, readFIFO);
	EMU->SetCPUWriteHandler(0x5, writeFIFO);
	EMU->SetCPUWriteHandler(0x9, writeReg);
	EMU->SetCPUWriteHandler(0xB, writeReg);
}

int16_t decodeADPCMNibble(uint8_t nibble) {
	static const int16_t diff_lookup[49*16] = {
		2, 6, 10, 14, 18, 22, 26, 30, -2, -6, -10, -14, -18, -22, -26, -30, 
		2, 6, 10, 14, 19, 23, 27, 31, -2, -6, -10, -14, -19, -23, -27, -31, 
		2, 6, 11, 15, 21, 25, 30, 34, -2, -6, -11, -15, -21, -25, -30, -34, 
		2, 7, 12, 17, 23, 28, 33, 38, -2, -7, -12, -17, -23, -28, -33, -38, 
		2, 7, 13, 18, 25, 30, 36, 41, -2, -7, -13, -18, -25, -30, -36, -41, 
		3, 9, 15, 21, 28, 34, 40, 46, -3, -9, -15, -21, -28, -34, -40, -46, 
		3, 10, 17, 24, 31, 38, 45, 52, -3, -10, -17, -24, -31, -38, -45, -52, 
		3, 10, 18, 25, 34, 41, 49, 56, -3, -10, -18, -25, -34, -41, -49, -56, 
		4, 12, 21, 29, 38, 46, 55, 63, -4, -12, -21, -29, -38, -46, -55, -63, 
		4, 13, 22, 31, 41, 50, 59, 68, -4, -13, -22, -31, -41, -50, -59, -68, 
		5, 15, 25, 35, 46, 56, 66, 76, -5, -15, -25, -35, -46, -56, -66, -76, 
		5, 16, 27, 38, 50, 61, 72, 83, -5, -16, -27, -38, -50, -61, -72, -83, 
		6, 18, 31, 43, 56, 68, 81, 93, -6, -18, -31, -43, -56, -68, -81, -93, 
		6, 19, 33, 46, 61, 74, 88, 101, -6, -19, -33, -46, -61, -74, -88, -101, 
		7, 22, 37, 52, 67, 82, 97, 112, -7, -22, -37, -52, -67, -82, -97, -112, 
		8, 24, 41, 57, 74, 90, 107, 123, -8, -24, -41, -57, -74, -90, -107, -123, 
		9, 27, 45, 63, 82, 100, 118, 136, -9, -27, -45, -63, -82, -100, -118, -136, 
		10, 30, 50, 70, 90, 110, 130, 150, -10, -30, -50, -70, -90, -110, -130, -150, 
		11, 33, 55, 77, 99, 121, 143, 165, -11, -33, -55, -77, -99, -121, -143, -165, 
		12, 36, 60, 84, 109, 133, 157, 181, -12, -36, -60, -84, -109, -133, -157, -181, 
		13, 39, 66, 92, 120, 146, 173, 199, -13, -39, -66, -92, -120, -146, -173, -199, 
		14, 43, 73, 102, 132, 161, 191, 220, -14, -43, -73, -102, -132, -161, -191, -220, 
		16, 48, 81, 113, 146, 178, 211, 243, -16, -48, -81, -113, -146, -178, -211, -243, 
		17, 52, 88, 123, 160, 195, 231, 266, -17, -52, -88, -123, -160, -195, -231, -266, 
		19, 58, 97, 136, 176, 215, 254, 293, -19, -58, -97, -136, -176, -215, -254, -293, 
		21, 64, 107, 150, 194, 237, 280, 323, -21, -64, -107, -150, -194, -237, -280, -323, 
		23, 70, 118, 165, 213, 260, 308, 355, -23, -70, -118, -165, -213, -260, -308, -355, 
		26, 78, 130, 182, 235, 287, 339, 391, -26, -78, -130, -182, -235, -287, -339, -391, 
		28, 85, 143, 200, 258, 315, 373, 430, -28, -85, -143, -200, -258, -315, -373, -430, 
		31, 94, 157, 220, 284, 347, 410, 473, -31, -94, -157, -220, -284, -347, -410, -473, 
		34, 103, 173, 242, 313, 382, 452, 521, -34, -103, -173, -242, -313, -382, -452, -521, 
		38, 114, 191, 267, 345, 421, 498, 574, -38, -114, -191, -267, -345, -421, -498, -574, 
		42, 126, 210, 294, 379, 463, 547, 631, -42, -126, -210, -294, -379, -463, -547, -631, 
		46, 138, 231, 323, 417, 509, 602, 694, -46, -138, -231, -323, -417, -509, -602, -694, 
		51, 153, 255, 357, 459, 561, 663, 765, -51, -153, -255, -357, -459, -561, -663, -765, 
		56, 168, 280, 392, 505, 617, 729, 841, -56, -168, -280, -392, -505, -617, -729, -841, 
		61, 184, 308, 431, 555, 678, 802, 925, -61, -184, -308, -431, -555, -678, -802, -925, 
		68, 204, 340, 476, 612, 748, 884, 1020, -68, -204, -340, -476, -612, -748, -884, -1020, 
		74, 223, 373, 522, 672, 821, 971, 1120, -74, -223, -373, -522, -672, -821, -971, -1120, 
		82, 246, 411, 575, 740, 904, 1069, 1233, -82, -246, -411, -575, -740, -904, -1069, -1233, 
		90, 271, 452, 633, 814, 995, 1176, 1357, -90, -271, -452, -633, -814, -995, -1176, -1357, 
		99, 298, 497, 696, 895, 1094, 1293, 1492, -99, -298, -497, -696, -895, -1094, -1293, -1492, 
		109, 328, 547, 766, 985, 1204, 1423, 1642, -109, -328, -547, -766, -985, -1204, -1423, -1642, 
		120, 360, 601, 841, 1083, 1323, 1564, 1804, -120, -360, -601, -841, -1083, -1323, -1564, -1804, 
		132, 397, 662, 927, 1192, 1457, 1722, 1987, -132, -397, -662, -927, -1192, -1457, -1722, -1987, 
		145, 436, 728, 1019, 1311, 1602, 1894, 2185, -145, -436, -728, -1019, -1311, -1602, -1894, -2185, 
		160, 480, 801, 1121, 1442, 1762, 2083, 2403, -160, -480, -801, -1121, -1442, -1762, -2083, -2403, 
		176, 528, 881, 1233, 1587, 1939, 2292, 2644, -176, -528, -881, -1233, -1587, -1939, -2292, -2644, 
		194, 582, 970, 1358, 1746, 2134, 2522, 2910, -194, -582, -970, -1358, -1746, -2134, -2522, -2910
	};
	static const int16_t index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
	
	signal += diff_lookup[step *16 + nibble];
	if (signal > 2047) signal = 2047;
	if (signal <-2048) signal =-2048;
	step +=index_shift[nibble &7];
	if (step > 48) step = 48;
	if (step <  0) step =  0;	
	return signal *16;
}

void MAPINT cpuCycle () {
	MMC3::cpuCycle();
	adpcmCount += rate;
	while (adpcmCount >= 1789772) {
		adpcmCount -= 1789772;
		if (adpcmNibble)
			adpcmOutput = decodeADPCMNibble(adpcmByte &0x0F);
		else {
			adpcmByte = fifo.retrieve();
			adpcmOutput = decodeADPCMNibble(adpcmByte >>4);
		}
		adpcmNibble = !adpcmNibble;
	}
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

int MAPINT mapperSound (int cycles) {
	return lowPassFilter.process(adpcmOutput +1e-15);
}

uint16_t mapperNum =594;
} // namespace

MapperInfo MapperInfo_594 = {
	&mapperNum,
	_T("Rinco FSG2"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	mapperSound,
	NULL
};
