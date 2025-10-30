#include "..\..\interface.h"
#include "s_LPC10.h"

#define LPC_FRAC_BITS 15

void LPC10::run() {
	chipCount +=chipClock;
	if (chipCount <hostClock)
		return;
	chipCount -=hostClock;

	int previousState =-1;
	while(state !=previousState) {
		if (waitCycles) {
			waitCycles--;
			return;
		}
		switch(previousState =state) {
		case 0:	case 1: case 2: case 3: case 4: case 5:
			if (variant->selectableCodecs.size()) {
				if (!haveBits(8)) break;
				codec =nullptr;
				bool foundAnything =false;
				for (auto& c: variant->selectableCodecs) {
					if ((unsigned) state <c.identifier.size() && gotBits ==c.identifier[state]) {
						state++;
						foundAnything =true;
						if ((unsigned) state ==c.identifier.size()) {
							codec =c.codec;
							state =101;
						}
					}
				}
				if (!foundAnything) state =0;
			} else {
				codec =variant->defaultCodec;
				state =101;
			}
			break;
		case 101: // Preload first frame
			returnState =102;
			getFrameSrc =&framePrev;
			getFrameDst =&framePrev;
			state =1000; // getFrame
			break;
		case 102: // Preload second frame
			returnState =103;
			getFrameSrc =&framePrev;
			getFrameDst =&frameNext;
			state =1000; // getFrame
			break;
		case 103: // Preload end
			frameCurr =framePrev;
			++state;
			break;
		case 104: // Process frame
			needInterpolation =!(
				 framePrev.pitch  && !frameNext.pitch  || // Old frame was voiced, new is unvoiced
				!framePrev.pitch  &&  frameNext.pitch  || // Old frame was unvoiced, new is voiced
				!framePrev.energy &&  frameNext.energy || // Old frame was silence/zero energy, new has non-zero energy
				!framePrev.pitch  && !frameNext.energy);  // Old frame was unvoiced, new frame is silence/zero energy (non-existent on tms51xx rev D and F (present and working on tms52xx, present but buggy on tms51xx rev A and B))*/
			state =200;
			break;
		case 200: case 201: case 202: case 203: case 204: case 205: case 206: case 207: case 208: case 209: case 210: case 211: case 212: case 213: case 214: case 215: case 216: case 217: case 218: case 219:
		case 220: case 221: case 222: case 223: case 224: case 225: case 226: case 227: case 228: case 229: case 230: case 231: case 232: case 233: case 234: case 235: case 236: case 237: case 238: case 239:
		case 240: case 241: case 242: case 243: case 244: case 245: case 246: case 247: case 248: case 249: case 250: case 251: case 252: case 253: case 254: case 255: case 256: case 257: case 258: case 259:
		case 260: case 261: case 262: case 263: case 264: case 265: case 266: case 267: case 268: case 269: case 270: case 271: case 272: case 273: case 274: case 275: case 276: case 277: case 278: case 279:
		case 280: case 281: case 282: case 283: case 284: case 285: case 286: case 287: case 288: case 289: case 290: case 291: case 292: case 293: case 294: case 295: case 296: case 297: case 298: case 299:
		case 300: case 301: case 302: case 303: case 304: case 305: case 306: case 307: case 308: case 309: case 310: case 311: case 312: case 313: case 314: case 315: case 316: case 317: case 318: case 319:
		case 320: case 321: case 322: case 323: case 324: case 325: case 326: case 327: case 328: case 329: case 330: case 331: case 332: case 333: case 334: case 335: case 336: case 337: case 338: case 339:
		case 340: case 341: case 342: case 343: case 344: case 345: case 346: case 347: case 348: case 349: case 350: case 351: case 352: case 353: case 354: case 355: case 356: case 357: case 358: case 359:
		case 360: case 361: case 362: case 363: case 364: case 365: case 366: case 367: case 368: case 369: case 370: case 371: case 372: case 373: case 374: case 375: case 376: case 377: case 378: case 379:
		case 380: case 381: case 382: case 383: case 384: case 385: case 386: case 387: case 388: case 389: case 390: case 391: case 392: case 393: case 394: case 395: case 396: case 397: case 398: case 399:
			sampleIndex =state -200;
			currentPitch -=16;
			if (currentPitch <0) reloadPitch();
			{
			short chirp =frameCurr.pitch? (currentPitch >=codec->pitchMax? 0: codec->chirpTable[currentPitch]): frameCurr.energy? (rng()? codec->unvoicedChirp: -codec->unvoicedChirp): 0;
			output =filter(codec->volume*((chirp *frameCurr.energy) >>LPC_FRAC_BITS));
			}
			++state;
			waitCycles =80;
			break;
		case 400:
			framePrev =frameNext;
			frameCurr =frameNext;
			returnState =104;
			getFrameSrc =&framePrev;
			getFrameDst =&frameNext;
			state =1000; // getFrame
			break;
		case 500:	// End of message reached
			if (variant->endCommandLength) {
				if (!haveBits(variant->endCommandLength)) break;
				if (gotBits !=variant->endCommandNumber) break;
			}
			state =0;
			break;
		/* Get frame data */
		case 1000: // Get Energy field
			if (haveBits(codec->energyBits)) {
				if (gotBits ==0) {
					getFrameDst->energy =0;
					getFrameDst->pitch =0;
					for (auto& c: getFrameDst->k) c =0;
					state =returnState;
				} else
				if (gotBits ==15) {
					while(fifo.size() &7) fifo.pop();
					state =500;
				} else {
					getFrameDst->energy =codec->energyTable[gotBits];
					++state;
				}
			}
			break;
		case 1001: // Get Repeat Field
			if (haveBits(1)) {
				getFrameRepeat =gotBits;
				++state;
			}
			break;
		case 1002: // Get Pitch field and evalute if K1-K4 are needed
			if (haveBits(codec->pitchBits)) {
				getFrameDst->pitch =codec->pitchTable[gotBits];
				if (getFrameRepeat) {
					for (int i =0; i <=3; i++) getFrameDst->k[i] =getFrameSrc->k[i];
					state =++state +4; // Skip requesting K1-K4 fields
				} else
					state =++state;
			}
			break;
		case 1003: // Get K1 field (index 0)
		case 1004: // Get K2 field (index 1)
		case 1005: // Get K3 field (index 2)
		case 1006: // Get K4 field (index 3)
			if (haveBits(codec->kBits[state -1003 +0])) {
				getFrameDst->k[state -1003 +0] =codec->kTable[state -1003 +0][gotBits];
				++state;
			}
			break;
		case 1007: // Evaluate if K5-K10 are needed
			if (getFrameRepeat) {
				for (int i =4; i <=9; i++) getFrameDst->k[i] =getFrameSrc->k[i];
				state =++state +6;
			} else
			if (getFrameDst->pitch ==0) {
				for (int i =4; i <=9; i++) getFrameDst->k[i] =0;
				state =++state +6;
			} else
				++state;
			break;
		case 1008: // Get K5  field (index 4)
		case 1009: // Get K6  field (index 5)
		case 1010: // Get K7  field (index 6)
		case 1011: // Get K8  field (index 7)
		case 1012: // Get K9  field (index 8)
		case 1013: // Get K10 field (index 9)
			if (haveBits(codec->kBits[state -1008 +4])) {
				getFrameDst->k[state -1008 +4] =codec->kTable[state -1008 +4][gotBits];
				++state;
			}
			break;
		case 1014: // Got all fields, return to caller
			state =returnState;
			break;
		case 9999: // reset
			state =0;
			break;
		default: // Invalid
			EMU->DbgOut(L"LPC10: Invalid state %d", state);
			break;
		}
	}
}

void LPC10::reloadPitch() {
	if (needInterpolation) {
		int ratio =(sampleIndex <<LPC_FRAC_BITS) /200;
		frameCurr.energy =framePrev.energy +(((frameNext.energy -framePrev.energy) *ratio) >>LPC_FRAC_BITS);
		frameCurr.pitch  =framePrev.pitch  +(((frameNext.pitch  -framePrev.pitch)  *ratio) >>LPC_FRAC_BITS);
		for (int i =0; i <10; i++) frameCurr.k[i] =framePrev.k[i] +(((frameNext.k[i] -framePrev.k[i]) *ratio) >>LPC_FRAC_BITS);
		currentPitch +=frameCurr.pitch;
		if (currentPitch >0) return;
	}
	if (frameCurr.pitch !=0) {
		currentPitch +=frameCurr.pitch;
		return;
	}
	currentPitch =0x80;
}

#define LPC_LIMIT 27500
int LPC10::filter(int sample) {
	for (int i =0; i <10; i++) {
		int index =10 -1 -i;
		sample -=(frameCurr.k[index] *filterX[index]) >>LPC_FRAC_BITS;
		if (sample >LPC_LIMIT)
			sample =LPC_LIMIT;
		else
		if (sample <-LPC_LIMIT)
			sample =-LPC_LIMIT;

		filterX[index + 1] =filterX[index] +((sample *frameCurr.k[index]) >>LPC_FRAC_BITS);
	}
	filterX[0] = (short) sample;
	return sample;
}

int LPC10::rng() {
	short seed =randomSeed <<1;
	short r =((seed >>12) ^(seed >>13)) &1;
	randomSeed =seed |r;
	return r;
}

bool LPC10::haveBits(unsigned int bits) {
	if (fifo.size() <bits) return false;
	gotBits =0;
	while (bits--) {
		gotBits =gotBits <<1 | (fifo.front()*1);
		fifo.pop();
	}
	return true;
}

LPC10::LPC10(Variant* _variant, int32_t _chipClock, int32_t _hostClock):
	variant(_variant),
	chipClock(_chipClock),
	hostClock(_hostClock),
	lowPassFilter(chipClock /80 *0.425 /hostClock) {
	reset();
}

void LPC10::reset() {
	framePrev.energy =framePrev.pitch =0; for (auto& c: framePrev.k) c =0;
	frameCurr.energy =frameCurr.pitch =0; for (auto& c: frameCurr.k) c =0;
	frameNext.energy =frameNext.pitch =0; for (auto& c: frameNext.k) c =0;
	needInterpolation =false;
	randomSeed =-1;
	currentPitch =sampleIndex =output =chipCount =waitCycles =returnState =gotBits =getFrameRepeat =0;
	for (auto& c: filterX) c =0;
	fifo =std::queue<bool>();
	state =9999; waitCycles =16000;
}

void LPC10::clearFIFO() {
	fifo =std::queue<bool>();
}

bool LPC10::getReadyState() const {
	return fifo.size() <LPC_FIFO_LENGTH/2 && state !=9999;
}

bool LPC10::getPlayingState() const {
	return state >=200 && state <=400;
}

bool LPC10::getFinishedState() const {
	return state ==500;
}

int LPC10::getAudio(int) {
	return lowPassFilter.process(output +1e-15);
}

void LPC10::writeBitsLSB(int bits, int val) {
	while (bits--) {
		if (fifo.size() >=LPC_FIFO_LENGTH) {
			EMU->DbgOut(L"LPC-10: FIFO overflow");
			return;
		}
		fifo.push(!!(val &1));
		val >>=1;
	}
}

void LPC10::writeBitsMSB(int bits, int val) {
	while (bits--) {
		if (fifo.size() >=LPC_FIFO_LENGTH) {
			EMU->DbgOut(L"LPC-10: FIFO overflow");
			return;
		}
		fifo.push(!!(val &0x80));
		val <<=1;
	}
}

int LPC10::saveLoad (STATE_TYPE stateMode, int stateOffset, unsigned char *stateData) {
	SAVELOAD_WORD(stateMode, stateOffset, stateData, framePrev.energy);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, framePrev.pitch);
	for (int i =0; i <10; i++) SAVELOAD_WORD(stateMode, stateOffset, stateData, framePrev.k[i]);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, frameCurr.energy);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, frameCurr.pitch);
	for (int i =0; i <10; i++) SAVELOAD_WORD(stateMode, stateOffset, stateData, frameCurr.k[i]);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, frameNext.energy);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, frameNext.pitch);
	for (int i =0; i <10; i++) SAVELOAD_WORD(stateMode, stateOffset, stateData, frameNext.k[i]);
	SAVELOAD_BOOL(stateMode, stateOffset, stateData, needInterpolation);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, randomSeed);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, currentPitch);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, sampleIndex);
	for (int i =0; i <11; i++) SAVELOAD_WORD(stateMode, stateOffset, stateData, filterX[i]);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, output);
	SAVELOAD_LONG(stateMode, stateOffset, stateData, chipCount);
	SAVELOAD_LONG(stateMode, stateOffset, stateData, waitCycles);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, state);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, returnState);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, gotBits);
	SAVELOAD_WORD(stateMode, stateOffset, stateData, getFrameRepeat);
	/* The mapper-specific state size must be constant, so save/load exactly LPC_FIFO_LENGTH FIFO bits along with the number of bits that were actually used. Avoid direct pointers as well. */
	if (stateMode ==STATE_LOAD) {
		while (!fifo.empty()) fifo.pop();
		uint32_t tempSize =0;
		SAVELOAD_LONG(stateMode, stateOffset, stateData, tempSize);
		if (tempSize >LPC_FIFO_LENGTH) EMU->DbgOut(L"LPC-10: More than %d FIFO bits; save state invalid", LPC_FIFO_LENGTH);
		for (int i =0; i <LPC_FIFO_LENGTH; i++) {
			bool tempData =false;
			SAVELOAD_BOOL(stateMode, stateOffset, stateData, tempData);
			if (tempSize >0) {
				fifo.push(tempData);
				tempSize--;
			}
		}
		/* "codec" is an absolute pointer that changes beween different memory allocations, so it cannot be directly saved/loaded.
		   Instead, save/load an index into the selectableCodecs vector and check its validity when loading. */
		if (variant->selectableCodecs.size()) {
			uint8_t found =0xFF;
			SAVELOAD_BYTE(stateMode, stateOffset, stateData, found);
			if (found !=0xFF && found <variant->selectableCodecs.size()) codec =variant->selectableCodecs[found].codec;
		} else
			codec =variant->defaultCodec;
		/* "getFrameSrc"/"getFrameDst" are absolute pointers that change between different memory allocations, so they cannot be saved/loaded.
		   Since they are only used during the GetFrame states (1000-1014), infer their correct content from the returnState that has been saved. */
		getFrameSrc =&framePrev;
		getFrameDst =returnState ==102? &framePrev: &frameNext;
	} else {
		auto fifo2 =fifo;
		uint32_t tempSize =fifo2.size();
		SAVELOAD_LONG(stateMode, stateOffset, stateData, tempSize);
		for (int i =0; i <LPC_FIFO_LENGTH; i++) {
			bool tempData =false;
			if (tempSize >0) {
				tempData =fifo2.front();
				fifo2.pop();
				tempSize--;
			}
			SAVELOAD_BOOL(stateMode, stateOffset, stateData, tempData);
		}
		if (variant->selectableCodecs.size()) {
			uint8_t found =0xFF;
			for (uint8_t i =0; i <variant->selectableCodecs.size(); i++) if (variant->selectableCodecs[i].codec ==codec) found =i;
			SAVELOAD_BYTE(stateMode, stateOffset, stateData, found);
		}
	}
	return stateOffset;
}

// LPC-D6 tables from TSP manual, appendix B; values pre-multiplied where appropriate
static const short d6_energy[] ={
	     0,   256,   512,   768,  1024,  1280,  1792,  2816,  4352,  6656, 10496, 16128, 21760, 28672, 32512,     0
};
static const short d6_pitchPeriod[] ={
	    0,  256,  260,  264,  272,  276,  280,  284,  292,  296,  300,  308,  312,  320,  324,  332,  336,  344,  348,  356,  364,  368,  376,  384,  388,  396,  404,  412,  420,  428,  436,  444,
	  452,  460,  468,  476,  484,  496,  504,  512,  524,  532,  540,  552,  560,  572,  584,  592,  604,  616,  628,  640,  652,  664,  676,  688,  700,  712,  724,  740,  752,  768,  780,  796,
	  808,  824,  840,  856,  872,  888,  904,  920,  936,  956,  972,  988, 1008, 1028, 1044, 1064, 1084, 1104, 1124, 1144, 1168, 1188, 1212, 1232, 1256, 1280, 1300, 1324, 1352, 1376, 1400, 1428,
	 1452, 1480, 1508, 1536, 1564, 1592, 1620, 1652, 1680, 1712, 1744, 1776, 1808, 1844, 1876, 1912, 1948, 1984, 2020, 2056, 2092, 2132, 2172, 2212, 2252, 2296, 2336, 2380, 2424, 2468, 2512, 2560
};
static const short d6_k1[] ={
	-32512,-32192,-31936,-31616,-31296,-30976,-30656,-30272,-29888,-29504,-29120,-28672,-28224,-27776,-27264,-26816,-26240,-25728,-25216,-24640,-24064,-23488,-22848,-22208,-21632,-20992,-20288,-19584,-18880,-18176,-17408,-16576,
	-15808,-14976,-14144,-13248,-12352,-11392,-10368, -9408, -8384, -7296, -6208, -5120, -4032, -2880, -1600,  -320,  1088,  2496,  3904,  5504,  7296,  9088, 10944, 12928, 14976, 17088, 19328, 21504, 23616, 25856, 28160, 30848
};
static const short d6_k2[] ={
	-30208,-26624,-23616,-21056,-19328,-17792,-16384,-15104,-13888,-12736,-11584,-10560, -9536, -8576, -7680, -6720, -5824, -4928, -4096, -3264, -2432, -1600,  -768,     0,   832,  1600,  2368,  3136,  3904,  4736,  5504,  6272,
	  7040,  7808,  8576,  9408, 10176, 10944, 11712, 12480, 13312, 14080, 14912, 15616, 16384, 17152, 17920, 18688, 19456, 20288, 21056, 21824, 22592, 23360, 24064, 24832, 25536, 26240, 26944, 27648, 28416, 29184, 30272, 31744
};
static const short d6_k3[] ={
	-29952,-26112,-24064,-22272,-20736,-19200,-17664,-16384,-15104,-13824,-12544,-11264, -9984, -8704, -7680, -6400, -5120, -3840, -2560, -1280,   256,  1792,  3328,  5120,  6656,  8704, 10496, 12800, 15104, 17664, 21248, 27904
};
static const short d6_k4[] ={
	-27648,-20480,-15872,-13568,-11520, -9984, -8448, -6912, -5632, -4352, -3072, -1792,  -512,   768,  1792,  3072,  4352,  5376,  6656,  7936,  9216, 10496, 11776, 13056, 14336, 15872, 17408, 19200, 21248, 23040, 25600, 29696
};
static const short d6_k5[] ={
	-23808,-15104,-11264, -8192, -5632, -3328, -1024,  1024,  3072,  5376,  7680,  9984, 12544, 15616, 19456, 26112
};
static const short d6_k6[] ={
	-22016,-10496, -6400, -3584, -1024,  1280,  3328,  5120,  7168,  9216, 11520, 13824, 16384, 18944, 21760, 27136
};
static const short d6_k7[] ={
	-23808,-14336,-10496, -7424, -4864, -2816,  -768,  1280,  3328,  5120,  7424,  9728, 12544, 15360, 19200, 26368
};
static const short d6_k8[] ={
	-15104, -7168, -2560,  1280,  5120,  9984, 15872, 22528
};
static const short d6_k9[] ={
	-18176, -9216, -5120, -1792,  1024,  4096,  7936, 17664
};
static const short d6_k10[] ={
	-15616, -6656, -3328,  -768,  1536,  4352,  7680, 17152
};
static const short d6_excitationFunction[] ={
	  162,  175,  186,  194,  199,  201,  202,  198,  194,  188,  181,  173,  165,  158,  154,  149,  149,  152,  159,  168,  184,  202,  227,  254,  287,  321,  361,  401,  445,  488,  534,  576,
	  620,  658,  697,  729,  760,  783,  805,  818,  831,  835,  839,  837,  837,  831,  829,  826,  829,  833,  846,  863,  891,  928,  978, 1037, 1111, 1197, 1297, 1410, 1536, 1674, 1823, 1981,
	 2148, 2321, 2497, 2676, 2854, 3029, 3199, 3360, 3511, 3648, 3771, 3876, 3962, 4028, 4073, 4095, 4095, 4073, 4028, 3962, 3876, 3771, 3648, 3511, 3360, 3199, 3029, 2854, 2676, 2497, 2321, 2148,
	 1981, 1823, 1674, 1536, 1410, 1297, 1197, 1111, 1037,  978,  928,  891,  863,  846,  833,  829,  826,  829,  831,  837,  837,  839,  835,  831,  818,  805,  783,  760,  729,  697,  658,  620,
	  576,  534,  488,  445,  401,  361,  321,  287,  254,  227,  202,  184,  168,  159,  152,  149,  149,  154,  158,  165,  173,  181,  188,  194,  198,  202,  201,  199,  194,  186,  175,  162
};

// LPC-PE tables from MAME's tms5110r.hxx; values pre-multiplied where appropriate
static const short pe_energy[] ={
	     0,   256,   512,   768,  1024,  1536,  2048,  2816,  4096,  5888,  8448, 12032, 16128, 21760, 29184,     0
};
static const short pe_pitchPeriod[] ={
	     0,   240,   256,   272,   288,   304,   320,   336,   352,   368,   384,   400,   416,   432,   448,   464,   480,   496,   512,   528,   544,   560,   576,   592,   608,   624,   640,   656,   672,   704,   736,   768,
	   800,   832,   848,   896,   928,   960,   992,  1040,  1088,  1120,  1152,  1216,  1248,  1280,  1344,  1376,  1456,  1504,  1568,  1616,  1680,  1744,  1824,  1888,  1952,  2032,  2112,  2192,  2272,  2368,  2448,  2544
};
static const short pe_k1[] ={
	-32064,-31872,-31808,-31680,-31552,-31424,-31232,-30848,-30592,-30336,-30016,-29696,-29376,-28928,-28480,-27968,-26368,-24320,-21696,-18432,-14528,-10112, -5184,   -64,  5120, 10048, 14464, 18368, 21568, 24256, 26304, 27904
};
static const short pe_k2[] ={
	-20992,-19392,-17536,-15616,-13504,-11200, -8832, -6336, -3776, -1152,  1536,  4096,  6720,  9152, 11520, 13760, 15872, 17792, 19584, 21184, 22656, 23936, 25088, 26112, 27008, 27840, 28480, 29120, 29632, 30080, 30464, 32384
};
static const short pe_k3[] ={
	-28224,-24768,-21312,-17856,-14400,-10944, -7488, -4032,  -576,  2880,  6272,  9728, 13184, 16640, 20096, 23552
};
static const short pe_k4[] ={
	-20992,-17472,-13888,-10304, -6784, -3200,   320,  3904,  7424, 11008, 14592, 18112, 21696, 25216, 28800, 32384
};
static const short pe_k5[] ={
	-20992,-18048,-15040,-12096, -9088, -6144, -3200,  -192,  2752,  5760,  8704, 11648, 14656, 17600, 20608, 23552
};
static const short pe_k6[] ={
	-16384,-13568,-10752, -7872, -5056, -2240,   640,  3456,  6272,  9152, 11968, 14848, 17664, 20480, 23360, 26176
};
static const short pe_k7[] ={
	-19712,-16640,-13568,-10496, -7488, -4416, -1344,  1728,  4800,  7808, 10880, 13952, 17024, 20096, 23104, 26176
};
static const short pe_k8[] ={
	-16384,-10304, -4224,  1856,  7936, 14016, 20096, 26176
};
static const short pe_k9[] ={
	-16384,-11264, -6144,  -960,  4160,  9344, 14464, 19648
};
static const short pe_k10[] ={
	-13120, -8448, -3776,   896,  5568, 10240, 14976, 19648
};

// LPC-PE decoder uses LPC-D6 excitation function for now
LPC10::Codec LPC10_PE ={ "PE",  8, 4, 6, { 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 }, 160, 1408, pe_energy, pe_pitchPeriod, d6_excitationFunction, { pe_k1, pe_k2, pe_k3, pe_k4, pe_k5, pe_k6, pe_k7, pe_k8, pe_k9, pe_k10 }};
LPC10::Codec LPC10_D6 ={ "D6", 12, 4, 7, { 6, 6, 5, 5, 4, 4, 4, 3, 3, 3 }, 160, 1408, d6_energy, d6_pitchPeriod, d6_excitationFunction, { d6_k1, d6_k2, d6_k3, d6_k4, d6_k5, d6_k6, d6_k7, d6_k8, d6_k9, d6_k10 }};

LPC10::Variant lpcGeneric ={ &LPC10_PE, {}, 0 };
LPC10::Variant lpcSubor   ={ &LPC10_D6, {{{0x50}, &LPC10_D6 }, {{0xD0}, &LPC10_PE }}, 8, 0xF0 };
LPC10::Variant lpcBBK     ={ &LPC10_D6, {{{0x6B}, &LPC10_D6 }}, 0 };
LPC10::Variant lpcABM     ={ &LPC10_PE, {{{0x52,0x0C,0x3F}, &LPC10_PE }}, 8, 0xFF };
