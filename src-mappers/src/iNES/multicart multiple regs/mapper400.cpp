#include "..\..\DLL\d_iNES.h"
#include "..\..\Hardware\h_Latch.h"

namespace {
uint8_t reg[4];
FCPUWrite writeRAM;
HWND hBitmapWindow;
uint8_t  ledData[160*1024];
uint32_t ledDataBytes;
uint8_t  ledCommand;
uint32_t ledBitmap[240*320];

void sync (void) {
	EMU->SetPRG_ROM16(0x8, Latch::data &7 | reg[2] &~7);
	EMU->SetPRG_ROM16(0xC,              7 | reg[3] &~7);
	EMU->SetCHR_RAM8(0, Latch::data >>5);
	if (reg[3] &0x20)
		EMU->Mirror_H();
	else
		EMU->Mirror_V();
}

void updateLED() {
	for (int i =0; i <320; i++) {
		for (int j =0; j <240; j++) {
			int packed =ledData[(i*240 +j) *2 +1] | ledData[(i*240 +j) *2 +0] <<8;
			int b =(packed >> 0 &0x1F) *255/0x1F;
			int g =(packed >> 5 &0x3F) *255/0x3F;
			int r =(packed >>11 &0x1F) *255/0x1F;
			ledBitmap[i *240 +(239 -j)] =r <<16 | g <<8 | b;
		}
	}
	InvalidateRect(hBitmapWindow, NULL, FALSE);
	UpdateWindow(hBitmapWindow);				
}

void MAPINT interceptRAMWrite (int bank, int addr, int val) {
	if (addr ==0xFF) {
		if (reg[1] ==0x00) {
			ledCommand =val;
			ledDataBytes =0;
		} else
		if (reg[1] ==0x02 && ledCommand ==0x2C) { // F4F0
			if (ledDataBytes <160*1024) ledData[ledDataBytes] =val;
			if (++ledDataBytes >=160*1024) ledDataBytes =0;
			if ((ledDataBytes %480) ==0) updateLED();
		}
	}
	writeRAM(bank, addr, val);
}

void MAPINT writeReg (int bank, int addr, int val) {
	if ((addr &0x7FF) ==0) { // Exclude 8 Bit Xmas 2008's writes to second APU (also at 7001-7015)
		reg[bank <<1 &2 | addr >>11 &1] =val;
		sync();
	}
}

void MAPINT writeLED (int bank, int addr, int val) {
	ROM->dipValue =val;
}

HDC bitmapDC;
BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), 240, 320, 1, 32, BI_RGB };

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBITMAP hBitmap, hDefaultBitmap;
	if (msg == WM_CREATE) {
		// Create 240x320 bitmap
		hBitmap =CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);        
		bitmapDC =CreateCompatibleDC(NULL);
		hDefaultBitmap =(HBITMAP) SelectObject(bitmapDC, hBitmap);
		SetDIBits(bitmapDC, hBitmap, 0, 320, ledBitmap, &bmi, DIB_RGB_COLORS);
	} else
	if (msg == WM_PAINT) {
		PAINTSTRUCT ps;
		HDC windowDC = BeginPaint(hwnd, &ps);
		SetDIBits(bitmapDC, hBitmap, 0, 320, ledBitmap, &bmi, DIB_RGB_COLORS);
		BitBlt(windowDC, 0, 0, 240, 320, bitmapDC, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
	} else
	if (msg == WM_DESTROY) {
		SelectObject(bitmapDC, hDefaultBitmap);
		DeleteDC(bitmapDC);
		DeleteObject(hBitmap);
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL MAPINT load (void) {
	Latch::load(sync, NULL);

	RECT mainRect;
	GetWindowRect(hWnd, &mainRect);
	
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc =WndProc;
	wc.hInstance =hInstance;
	wc.lpszClassName =L"BitmapWindow";
	RegisterClass(&wc);
	hBitmapWindow =CreateWindow(L"BitmapWindow", L"LED Cartridge Label", WS_OVERLAPPEDWINDOW, mainRect.right, mainRect.top, 256, 358, hWnd, NULL, hInstance, NULL);    
	ShowWindow(hBitmapWindow, SW_NORMAL);
	UpdateWindow(hBitmapWindow);
	return TRUE;
}

void MAPINT reset (RESET_TYPE resetType) {
	reg[2] =0x00;
	reg[3] =0x80;
	Latch::reset(RESET_HARD);
	writeRAM =EMU->GetCPUWriteHandler(0x0);
	EMU->SetCPUWriteHandler(0x0, interceptRAMWrite);
	for (int bank =0x6; bank<=0x7; bank++) EMU->SetCPUWriteHandler(bank, writeReg);
	for (int bank =0x8; bank<=0xB; bank++) EMU->SetCPUWriteHandler(bank, writeLED);
	ledDataBytes =0;
	memset(ledData, 0xFF, sizeof(ledData));
	updateLED();
}

void MAPINT unload () {
	CloseWindow(hBitmapWindow);
	UnregisterClass(L"BitmapWindow", hInstance);
}

int MAPINT saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =Latch::saveLoad_D(stateMode, offset, data);
	for (auto& c: reg) SAVELOAD_BYTE(stateMode, offset, data, c);
	if (stateMode ==STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =400;
} // namespace

MapperInfo MapperInfo_400 ={
	&mapperNum,
	_T("8BIT-XMAS"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	NULL,
	NULL,
	saveLoad,
	NULL,
	NULL
};