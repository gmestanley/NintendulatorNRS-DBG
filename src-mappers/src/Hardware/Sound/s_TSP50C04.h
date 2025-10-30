/*******************************************************************************
 *
 *  LPC-10 D6 Synthesizer
 *
 *  Author:  <87430545@qq.com>
 *
 *  Create:  Oct/31/2021 by fanoble
 *
 *******************************************************************************
 */

#pragma once

#include <queue>

#define LPC_ORDER 10

class TSP50C04 {
	struct LPC_FRAME {
		short energy;
		short pitch;
		short k[LPC_ORDER]; // K1-K10
	};
	LPC_FRAME frame_prev; // 0
	LPC_FRAME frame_curr; // 2
	LPC_FRAME frame_next; // 1

	short need_interp;
	short random_seed;
	short curr_pitch;
	short sample_index; // from 0 to LPC_SAMPLES_PER_FRAME - 1

	short x[LPC_ORDER + 1]; // filter buf
	short synth_out; // output sample

	int state;

	const bool expectCodecByte;
	bool chinese;

	short	get_nbits(short);
	int	get_frame(LPC_FRAME*, LPC_FRAME*);
	void	set_interp_flag();
	void	preload();
	short	reload_pitch();
	void	synth_do_filter();
	short	random_gen();
	int	synth_run();
	int	synth_do();
	void	synth_reset();
public:
	TSP50C04(bool);
void	reset();
uint8_t	readSubor();
uint8_t	readOther();
void	write4Bits(uint8_t);
void	write8Bits(uint8_t);
void	writeBit(bool);
int16_t	getAudio(bool);

	std::queue<bool>    input;
	std::queue<int16_t> output;
};

