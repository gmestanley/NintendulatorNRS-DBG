#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"

#include "..\..\interface.h"
#include "s_MP3.h"

namespace MP3Player {
static mp3dec_t mp3d;
static mp3dec_file_info_t info;
bool ready =false;
bool playing =false;
uint32_t position;
uint32_t fetchCount;

void loadSong(TCHAR *name) {
	if (ready) freeSong();
	if (!mp3dec_load_w(&mp3d, name, &info, NULL, NULL)) {
		if (info.channels >0) {
			ready =true;
			position =0;
			fetchCount =0;
			EMU->DbgOut(L"Loaded, %d Hz, %d channels, %d samples", info.hz, info.channels, info.samples);
		}
	} else
		EMU->DbgOut(L"Fail!");
}

void freeSong() {
	if (ready) {
		ready =false;
		free(info.buffer);
		info.buffer =NULL;
	}
}

void start() {
	playing =true;
}

void stop() {
	playing =false;
}

int getNextSample() {
	int result =0;
	
	bool finished =position >=info.samples;
	if (playing && !finished) {
		for (int i =0; i <info.channels; i++) result +=info.buffer[position +i];
		fetchCount +=info.hz;
		while (fetchCount >=1789773) {
			position +=info.channels;
			fetchCount -=1789773;
		}
		finished =position >=info.samples;
		result /=info.channels;
	}
	return result;
}

uint32_t getPosition() {
	return position;
}

void setPosition(uint32_t newPosition) {
	if (newPosition <info.samples)
		position =newPosition;
	else
		position=info.samples;
}

}
