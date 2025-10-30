#pragma once

namespace MP3Player {
void loadSong(TCHAR *);
void freeSong();
void start();
void stop();
int getNextSample();
uint32_t getPosition();
void setPosition(uint32_t);
}
