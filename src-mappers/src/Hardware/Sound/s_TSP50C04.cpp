/*******************************************************************************
 *
 *  LPC-10 D6 Synthesizer
 *
 *  Author:  <87430545@qq.com>
 *
 *  Create:  May/6/2021 by fanoble
 *
 *******************************************************************************
 */
#include "..\..\interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "s_TSP50C04.h"

#define LPC_SAMPLES_PER_FRAME 200 // speed
#define LPC_FRAC_BITS 15 // using Q15
#define LPC_STATE_RESET			0
#define LPC_STATE_STARTUP		1
#define LPC_STATE_RUN			2
#define LPC_STATE_FINISHED		3
#define LPC_STATE_STOPPED		4

#pragma warning (disable:4305)
#pragma warning (disable:4309)
#pragma warning (disable:4838)

static const short d6_energy[] ={ // From TSP manual page B-33
	     0,   256,   512,   768,  1024,  1280,  1792,  2816,
	  4352,  6656, 10496, 16128, 21760, 28672, 32512,     0
};

static const short d6_pitchPeriod[] ={ // From TSP manual page B-33, first value replaced with 0
	    0,  256,  260,  264,  272,  276,  280,  284,
	  292,  296,  300,  308,  312,  320,  324,  332,
	  336,  344,  348,  356,  364,  368,  376,  384,
	  388,  396,  404,  412,  420,  428,  436,  444,
	  452,  460,  468,  476,  484,  496,  504,  512,
	  524,  532,  540,  552,  560,  572,  584,  592,
	  604,  616,  628,  640,  652,  664,  676,  688,
	  700,  712,  724,  740,  752,  768,  780,  796,
	  808,  824,  840,  856,  872,  888,  904,  920,
	  936,  956,  972,  988, 1008, 1028, 1044, 1064,
	 1084, 1104, 1124, 1144, 1168, 1188, 1212, 1232,
	 1256, 1280, 1300, 1324, 1352, 1376, 1400, 1428,
	 1452, 1480, 1508, 1536, 1564, 1592, 1620, 1652,
	 1680, 1712, 1744, 1776, 1808, 1844, 1876, 1912,
	 1948, 1984, 2020, 2056, 2092, 2132, 2172, 2212,
	 2252, 2296, 2336, 2380, 2424, 2468, 2512, 2560
};

static const short d6_k1[] ={ // From TSP manual page B-36
	-32512,-32192,-31936,-31616,-31296,-30976,-30656,-30272,
	-29888,-29504,-29120,-28672,-28224,-27776,-27264,-26816,
	-26240,-25728,-25216,-24640,-24064,-23488,-22848,-22208,
	-21632,-20992,-20288,-19584,-18880,-18176,-17408,-16576,
	-15808,-14976,-14144,-13248,-12352,-11392,-10368, -9408,
	 -8384, -7296, -6208, -5120, -4032, -2880, -1600,  -320,
	  1088,  2496,  3904,  5504,  7296,  9088, 10944, 12928,
	 14976, 17088, 19328, 21504, 23616, 25856, 28160, 30848
};

static const short d6_k2[] ={ // From TSP manual page B-38
	-30208,-26624,-23616,-21056,-19328,-17792,-16384,-15104,
	-13888,-12736,-11584,-10560, -9536, -8576, -7680, -6720,
	 -5824, -4928, -4096, -3264, -2432, -1600,  -768,     0,
	   832,  1600,  2368,  3136,  3904,  4736,  5504,  6272,
	  7040,  7808,  8576,  9408, 10176, 10944, 11712, 12480,
	 13312, 14080, 14912, 15616, 16384, 17152, 17920, 18688,
	 19456, 20288, 21056, 21824, 22592, 23360, 24064, 24832,
	 25536, 26240, 26944, 27648, 28416, 29184, 30272, 31744
};

static const short d6_k3[] ={ // From TSP manual page B-39
	-29952,-26112,-24064,-22272,-20736,-19200,-17664,-16384,
	-15104,-13824,-12544,-11264, -9984, -8704, -7680, -6400,
	 -5120, -3840, -2560, -1280,   256,  1792,  3328,  5120,
	  6656,  8704, 10496, 12800, 15104, 17664, 21248, 27904
};

static const short d6_k4[] ={ // From TSP manual page B-39
	-27648,-20480,-15872,-13568,-11520, -9984, -8448, -6912,
	 -5632, -4352, -3072, -1792,  -512,   768,  1792,  3072,
	  4352,  5376,  6656,  7936,  9216, 10496, 11776, 13056,
	 14336, 15872, 17408, 19200, 21248, 23040, 25600, 29696
};

static const short d6_k5[] ={ // From TSP manual page B-39
	-23808,-15104,-11264, -8192, -5632, -3328, -1024,  1024,
	  3072,  5376,  7680,  9984, 12544, 15616, 19456, 26112
};

static const short d6_k6[] ={ // From TSP manual page B-39
	-22016,-10496, -6400, -3584, -1024,  1280,  3328,  5120,
	  7168,  9216, 11520, 13824, 16384, 18944, 21760, 27136
};

static const short d6_k7[] ={ // From TSP manual page B-40
	-23808,-14336,-10496, -7424, -4864, -2816,  -768,  1280,
	  3328,  5120,  7424,  9728, 12544, 15360, 19200, 26368
};

static const short d6_k8[] ={ // From TSP manual page B-40
	-15104, -7168, -2560,  1280,  5120,  9984, 15872, 22528
};

static const short d6_k9[] ={ // From TSP manual page B-40
	-18176, -9216, -5120, -1792,  1024,  4096,  7936, 17664
};

static const short d6_k10[] ={ // From TSP manual page B-40
	-15616, -6656, -3328,  -768,  1536,  4352,  7680, 17152
};

static const short d6_excitationFunction[] ={
	  162,  175,  186,  194,  199,  201,  202,  198,
	  194,  188,  181,  173,  165,  158,  154,  149,
	  149,  152,  159,  168,  184,  202,  227,  254,
	  287,  321,  361,  401,  445,  488,  534,  576,
	  620,  658,  697,  729,  760,  783,  805,  818,
	  831,  835,  839,  837,  837,  831,  829,  826,
	  829,  833,  846,  863,  891,  928,  978, 1037,
	 1111, 1197, 1297, 1410, 1536, 1674, 1823, 1981,
	 2148, 2321, 2497, 2676, 2854, 3029, 3199, 3360,
	 3511, 3648, 3771, 3876, 3962, 4028, 4073, 4095,
	 4095, 4073, 4028, 3962, 3876, 3771, 3648, 3511,
	 3360, 3199, 3029, 2854, 2676, 2497, 2321, 2148,
	 1981, 1823, 1674, 1536, 1410, 1297, 1197, 1111,
	 1037,  978,  928,  891,  863,  846,  833,  829,
	  826,  829,  831,  837,  837,  839,  835,  831,
	  818,  805,  783,  760,  729,  697,  658,  620,
	  576,  534,  488,  445,  401,  361,  321,  287,
	  254,  227,  202,  184,  168,  159,  152,  149,
	  149,  154,  158,  165,  173,  181,  188,  194,
	  198,  202,  201,  199,  194,  186,  175,  162
};

static const short pe_energy[] ={
	     0,   256,   512,   768,  1024,  1536,  2048,  2816,
	  4096,  5888,  8448, 12032, 16128, 21760, 29184,     0
};

static const short pe_pitchPeriod[] ={
	     0,   240,   256,   272,   288,   304,   320,   336,
	   352,   368,   384,   400,   416,   432,   448,   464,
	   480,   496,   512,   528,   544,   560,   576,   592,
	   608,   624,   640,   656,   672,   704,   736,   768,
	   800,   832,   848,   896,   928,   960,   992,  1040,
	  1088,  1120,  1152,  1216,  1248,  1280,  1344,  1376,
	  1456,  1504,  1568,  1616,  1680,  1744,  1824,  1888,
	  1952,  2032,  2112,  2192,  2272,  2368,  2448,  2544
};

static const short pe_k1[] ={
	-32064,-31872,-31808,-31680,-31552,-31424,-31232,-30848,
	-30592,-30336,-30016,-29696,-29376,-28928,-28480,-27968,
	-26368,-24320,-21696,-18432,-14528,-10112, -5184,   -64,
	  5120, 10048, 14464, 18368, 21568, 24256, 26304, 27904
};
static const short pe_k2[] ={
	-20992,-19392,-17536,-15616,-13504,-11200, -8832, -6336,
	 -3776, -1152,  1536,  4096,  6720,  9152, 11520, 13760,
	 15872, 17792, 19584, 21184, 22656, 23936, 25088, 26112,
	 27008, 27840, 28480, 29120, 29632, 30080, 30464, 32384
};
static const short pe_k3[] ={
	-28224,-24768,-21312,-17856,-14400,-10944, -7488, -4032,
	  -576,  2880,  6272,  9728, 13184, 16640, 20096, 23552
};
static const short pe_k4[] ={
	-20992,-17472,-13888,-10304, -6784, -3200,   320,  3904,
	  7424, 11008, 14592, 18112, 21696, 25216, 28800, 32384
};
static const short pe_k5[] ={
	-20992,-18048,-15040,-12096, -9088, -6144, -3200,  -192,
	  2752,  5760,  8704, 11648, 14656, 17600, 20608, 23552
};
static const short pe_k6[] ={
	-16384,-13568,-10752, -7872, -5056, -2240,   640,  3456,
	  6272,  9152, 11968, 14848, 17664, 20480, 23360, 26176
};
static const short pe_k7[] ={
	-19712,-16640,-13568,-10496, -7488, -4416, -1344,  1728,
	  4800,  7808, 10880, 13952, 17024, 20096, 23104, 26176
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

short TSP50C04::get_nbits(short bits) { // bits <= 8
	short result =0;
	while (bits--) {
		if (input.empty()) {
			state =LPC_STATE_STOPPED;
			//EMU->DbgOut(L"LPC: Stopped");
			return -1;
		}
		result =result <<1 | input.front() *1;
		input.pop();
	}
	return result;
}

int TSP50C04::get_frame(LPC_FRAME* dst, LPC_FRAME* ref) {
	int pitch_i;
	int energy_i;
	int repeat;

	energy_i = get_nbits(4);
	if (0 == energy_i) {
		// silent
		dst->energy = 0;
		dst->pitch = 0;
		memset(dst->k, 0, sizeof(ref->k));
		return 0;
	}

	// end of stream
	if (15 == energy_i)
		return -1;

	repeat = get_nbits(1); // repeat
	pitch_i = get_nbits(chinese? 7: 6); // pitch

	// coded -> uncoded
	dst->energy = chinese? d6_energy[energy_i]: pe_energy[energy_i];
	dst->pitch  = chinese? d6_pitchPeriod[pitch_i]: pe_pitchPeriod[pitch_i];

	if (repeat) {
		dst->k[0] = ref->k[0];
		dst->k[1] = ref->k[1];
		dst->k[2] = ref->k[2];
		dst->k[3] = ref->k[3];
	} else {
		dst->k[0] = chinese? d6_k1[get_nbits(6)]: pe_k1[get_nbits(5)]; // K1
		dst->k[1] = chinese? d6_k2[get_nbits(6)]: pe_k2[get_nbits(5)]; // K2
		dst->k[2] = chinese? d6_k3[get_nbits(5)]: pe_k3[get_nbits(4)]; // K3
		dst->k[3] = chinese? d6_k4[get_nbits(5)]: pe_k4[get_nbits(4)]; // K4
	}

	if (0 == pitch_i) {
		// lpc unvoiced frame
		dst->k[4] = 0;
		dst->k[5] = 0;
		dst->k[6] = 0;
		dst->k[7] = 0;
		dst->k[8] = 0;
		dst->k[9] = 0;
	} else {
		// voiced
		if (repeat) {
			memcpy(dst->k, ref->k, sizeof(ref->k));
		} else {
			dst->k[4] = chinese? d6_k5 [get_nbits(4)]: pe_k5 [get_nbits(4)];// K5
			dst->k[5] = chinese? d6_k6 [get_nbits(4)]: pe_k6 [get_nbits(4)];// K6
			dst->k[6] = chinese? d6_k7 [get_nbits(4)]: pe_k7 [get_nbits(4)];// K7
			dst->k[7] = chinese? d6_k8 [get_nbits(3)]: pe_k8 [get_nbits(3)];// K8
			dst->k[8] = chinese? d6_k9 [get_nbits(3)]: pe_k9 [get_nbits(3)];// K9
			dst->k[9] = chinese? d6_k10[get_nbits(3)]: pe_k10[get_nbits(3)];// K10
		}
	}

	return 0;
}

void TSP50C04::set_interp_flag() {
	need_interp = 1;

	if (frame_prev.energy == 0) {
		if (frame_next.pitch == 0) {
			if (frame_next.energy)
				need_interp = 0;
		}
	} else {
		if (frame_prev.pitch) {
			if (frame_next.pitch == 0) {
				if (frame_next.energy)
					need_interp = 0;
			}
		} else {
			if (frame_next.pitch)
				need_interp = 0;
		}
	}
}

void TSP50C04::preload() {
	chinese =false;
	if (expectCodecByte) {
		uint8_t codec =get_nbits(8);
		//EMU->DbgOut(L"Codec: %02X", codec);
		switch(codec) {
		case 0x50:
		case 0xD6: chinese =true; break;
		default: chinese =false; break;
		}
	}
	state = LPC_STATE_RUN;
	//EMU->DbgOut(L"LPC: Run");
	get_frame(&frame_prev, &frame_prev);
	get_frame(&frame_next, &frame_prev);
	frame_curr = frame_prev;
	set_interp_flag();
}

short TSP50C04::reload_pitch() {
	short cp;

	cp = curr_pitch;

	if (need_interp) {
		int i;
		int ratio;
		short* vector0;
		short* vector1;
		short* vector2;

		ratio = (sample_index << LPC_FRAC_BITS) / LPC_SAMPLES_PER_FRAME;

		vector0 = (short*)&frame_prev;
		vector1 = (short*)&frame_next;
		vector2 = (short*)&frame_curr;

		// do interp
		for (i = 0; i < sizeof(LPC_FRAME) / sizeof(short); i++) {
			short v;

			v = vector1[i] - vector0[i];
			v = (short)((v * ratio) >> LPC_FRAC_BITS);
			vector2[i] = v + vector0[i];
		}

		cp += frame_curr.pitch;
		if (cp > 0)
			return cp;
	}

	if (frame_curr.pitch != 0) {
		cp += frame_curr.pitch;
		return cp;
	}

	return 0x80;
}

#define LPC_CLAMP_P 27500 // to be fixed
#define LPC_CLAMP_N -(LPC_CLAMP_P + 1)

void TSP50C04::synth_do_filter() {
	int i;
	int sample;
	short* k;

	k = frame_curr.k;
	sample = synth_out;

	for (i = 0; i < LPC_ORDER; i++) {
		int index;

		index = LPC_ORDER - 1 - i;

		sample -= (k[index] * x[index]) >> LPC_FRAC_BITS;

		if (sample > LPC_CLAMP_P)
			sample = LPC_CLAMP_P;
		else if (sample < LPC_CLAMP_N)
			sample = LPC_CLAMP_N;

		x[index + 1] = x[index] + ((sample * k[index]) >> LPC_FRAC_BITS);
	}

	x[0] = synth_out = (short)sample;
}

short TSP50C04::random_gen() {
	short r;
	short seed;

	r = 0;

	seed = random_seed << 1;

	r = ((seed >> 12) ^ (seed >> 13)) & 1;

	random_seed = seed | r;

	return r;
}

int TSP50C04::synth_run() {
	int i;
	int excit;
	int eos;

	if (LPC_STATE_STOPPED == state) return 1;

	excit = 0;
	eos =0;

	for (i = 0; i < LPC_SAMPLES_PER_FRAME; i++) {
		sample_index = i;
		
		curr_pitch -=16;
		if (curr_pitch < 0) curr_pitch = reload_pitch();

		if (0 == frame_curr.pitch) {
			// unvoiced
			if (frame_curr.energy) {
				excit = random_gen()? 1408: -1408;
				excit = (excit * frame_curr.energy) >> LPC_FRAC_BITS;
			} else {
				// silent
				excit = 0;
			}
		} else if (curr_pitch >=160) {
			excit = 0;
		} else {
			// lpc voiced excit
			excit = d6_excitationFunction[curr_pitch];
			excit = (excit * frame_curr.energy) >> LPC_FRAC_BITS;
		}
		excit *= 8;

		// setup lpc
		synth_out = (short)excit;

		synth_do_filter();

		output.push(synth_out);
	}

	// prepare for the next frame
	frame_prev = frame_next;
	frame_curr = frame_next;

	if (get_frame(&frame_next, &frame_prev)) {
		// end of stream
		eos = 1;
		state = LPC_STATE_FINISHED;
		//EMU->DbgOut(L"LPC: Finished");
	}

	if (LPC_STATE_RESET == state) {
		// do not end stream for SB2K
		eos =0;
	}

	set_interp_flag();

	return eos;
}

int TSP50C04::synth_do() {
	if (LPC_STATE_FINISHED == state) {
		if (0xF0 == get_nbits(8)) {
			state = LPC_STATE_RESET;
			//EMU->DbgOut(L"LPC: Reset");
		}
		return 0;
	}
	if (LPC_STATE_RESET ==state) synth_reset();
	if (LPC_STATE_STARTUP ==state) {
		preload();
		return 0;
	}
	return synth_run();
}

void TSP50C04::synth_reset() {
	for (auto& c: x) c =0;
	frame_prev.energy =frame_prev.pitch =0; for (auto& c: frame_prev.k) c =0;
	frame_curr.energy =frame_curr.pitch =0; for (auto& c: frame_curr.k) c =0;
	frame_next.energy =frame_next.pitch =0; for (auto& c: frame_next.k) c =0;
	need_interp =curr_pitch =sample_index =synth_out =0;
	state =LPC_STATE_STARTUP;
	//if (EMU) EMU->DbgOut(L"LPC: Startup");
	random_seed =0xFFFF;
}

uint8_t TSP50C04::readSubor() {
	if (state ==LPC_STATE_FINISHED && !output.empty())
		return 0x00;
	else
	if (state ==LPC_STATE_FINISHED && output.empty())
		return 0x8F;
	else
	if (input.size() >=16*8)
		return 0x00;
	else
		return 0x80;
}

uint8_t TSP50C04::readOther() {
	if (state ==LPC_STATE_FINISHED && !output.empty())
		return 0x00;
	else
	if (state ==LPC_STATE_FINISHED && output.empty())
		return 0x40;
	else
	if (state ==LPC_STATE_RUN && output.size() >200)
		return 0x00;
	else
		return 0x40;
}

void TSP50C04::write8Bits(uint8_t val) {
	if ((state ==LPC_STATE_FINISHED) && val ==0x0F) {
		reset();
		return;
	}
	for (int bit =0; bit <8; bit++) {
		input.push(!!(val &1));
		val >>=1;
	}
	if ((state ==LPC_STATE_STOPPED || state ==LPC_STATE_FINISHED) && input.size() ==16*8) synth_reset();
}

void TSP50C04::write4Bits(uint8_t val) {
	if ((val &0xF0) ==0) {
		//EMU->DbgOut(L"Reset4 because val=%02X", val);		
		synth_reset();
		input =std::queue<bool>();
		return;
	}
	for (int bit =0; bit <4; bit++) {
		input.push(!!(val &1));
		val >>=1;
	}
	if ((state ==LPC_STATE_STOPPED || state ==LPC_STATE_FINISHED) && input.size() ==16*8) synth_reset();
}

void TSP50C04::writeBit(bool bit) {
	EMU->DbgOut(L"bit %d", bit);
	input.push(bit);
	if ((state ==LPC_STATE_STOPPED || state ==LPC_STATE_FINISHED) && input.size() ==16*8) synth_reset();
}

int16_t TSP50C04::getAudio(bool pop) {
	int16_t result =0;
	if (!output.empty()) {
		result =output.front();
		if (pop) output.pop();
	}
	if (input.size() >=16*8) synth_do();
	return result;
}

void TSP50C04::reset() {
	input =std::queue<bool>();
	output =std::queue<int16_t>();
	synth_reset();
}

TSP50C04::TSP50C04(bool _expectCodecByte): expectCodecByte(_expectCodecByte) {
	reset();
}

/*
SB97 access:
	uint8_t lpcState;
	while (~(lpcState =read(0x5300)) &0x80);
	write(0x5300, codec);
	while(1) {
		while (~(lpcState =read(0x5300)) &0x80);
		if ((lpcState &0x7F) ==0x0F) break;
		write(0x5300, dataByte);
	}
	write(0x5300, 0x0F);
*/