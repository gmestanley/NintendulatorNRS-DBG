#include	"..\..\DLL\d_iNES.h"
#include	"..\..\Hardware\h_MMC3.h"

namespace {
uint8_t		prg[4];
uint8_t		keyboardRow;

static const uint8_t keyMap[13][8] ={ // Same as Subor keyboard
	{DIK_4,		DIK_G,		DIK_F,		DIK_C,		DIK_F2,		DIK_E,		DIK_5,		DIK_V},
	{DIK_2,		DIK_D,		DIK_S,		DIK_END,	DIK_F1,		DIK_W,		DIK_3,		DIK_X},
	{DIK_INSERT,	DIK_BACKSPACE,	DIK_PGDN,	DIK_RIGHT,	DIK_F8,		DIK_PGUP,	DIK_DELETE,	DIK_HOME},
	{DIK_9,		DIK_I,		DIK_L,		DIK_COMMA,	DIK_F5,		DIK_O,		DIK_0,		DIK_PERIOD},
	{DIK_RBRACKET,	DIK_RETURN,	DIK_UP,		DIK_LEFT,	DIK_F7,		DIK_LBRACKET,	DIK_BACKSLASH,	DIK_DOWN},
	{DIK_Q,		DIK_CAPITAL,	DIK_Z,		DIK_TAB,	DIK_ESCAPE,	DIK_A,		DIK_1,		DIK_LCONTROL},
	{DIK_7,		DIK_Y,		DIK_K,		DIK_M,		DIK_F4,		DIK_U,		DIK_8,		DIK_J},
	{DIK_MINUS,	DIK_SEMICOLON,	DIK_APOSTROPHE,	DIK_SLASH,	DIK_F6,		DIK_P,		DIK_EQUALS,	DIK_LSHIFT},
	{DIK_T,		DIK_H,		DIK_N,		DIK_SPACE,	DIK_F3,		DIK_R,		DIK_6,		DIK_B},
	{DIK_GRAVE,	DIK_F10,	DIK_F11,	DIK_F12,	DIK_ADD,	DIK_SUBTRACT,	DIK_MULTIPLY,	DIK_DIVIDE},
	{DIK_SCROLL,	DIK_PAUSE,	DIK_GRAVE,	DIK_TAB,	DIK_NUMPAD6,	DIK_NUMPAD7,	DIK_NUMPAD8,	DIK_NUMPAD9},
	{DIK_CAPITAL,	DIK_SLASH,	DIK_RSHIFT,	DIK_LMENU,	DIK_F9,		DIK_ADD,	DIK_SUBTRACT,	DIK_MULTIPLY},
	{DIK_RMENU,	DIK_APPS,	DIK_RCONTROL,	DIK_NUMPAD1,	DIK_DIVIDE,	DIK_DECIMAL,	DIK_NUMLOCK,	DIK_NUMPADENTER}
};

void	sync (void) {
	MMC3::syncMirror();
	MMC3::syncWRAM(0);
	MMC3::syncPRG(0xFF, 0);
	MMC3::syncCHR(0xFF, 0);
}

int getPRGBank(int bank) {
	return MMC3::getPRGBank(bank) &0x1F | prg[bank] &0xE0;
}

int	MAPINT	readReg (int bank, int addr) {
	int keys =0;
	switch (addr) {
		case 0x906:
			if (keyboardRow <13) for (int keyboardColumn =0; keyboardColumn <8; keyboardColumn++) keys =keys >>1 | EMU->keyState[keyMap[keyboardRow][keyboardColumn]] &0x80;
			return ~keys;
		case 0xC03: // LPC chip status
			return 0;
		default:
			return EMU->ReadAPU(bank, addr);
			break;
	}
}

void	MAPINT	writeReg (int bank, int addr, int val) {
	EMU->WriteAPU(bank, addr, val);
	switch (addr) {
		case 0x904:
			keyboardRow =val;
			break;
		case 0x905:
			// Keyboard, written to at end of VBlank handler
			break;
		case 0xC00: case 0xC01:
			prg[addr &3] =val &0xE0;
			sync();
			break;
		case 0xC04: case 0xC05: case 0xC06: case 0xC07: // LPC chip
		/*
			1 bit: 4C07->4C05,4C04
			0 bit: 4C06->4C05
			
			0,0
			12 bits, value: 0A
			4 bits, value: 06
			0,1
			
			0,0
			4 bits
			8 bits
			4 bits, value 6
			0,1
			
			addr &1: SDA
			addr &2: SCL
			
			Start condition (E386)
			Start of message: Start condition, 12 bit: 0A, 4 bits: 06, done
			Message byte: Start condition, 4 bit: lower nibble, 4bit: upper nibble?, 06 done
			7 bit
			
		*/
			/*if (lastAddr &2 && ~addr &2) lpc.writeBit(!!(lastAddr &1));
			lastAddr =addr;*/
			break;
	}
}

BOOL	MAPINT	load (void) {
	MMC3::load(sync, MMC3Type::AX5202P, getPRGBank, NULL, NULL, NULL);
	iNES_SetSRAM();
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) for (auto& c: prg) c =0xE0;
	MMC3::reset(resetType);
	EMU->SetCPUReadHandler(0x4, readReg);
	EMU->SetCPUWriteHandler(0x4, writeReg);
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =MMC3::saveLoad(stateMode, offset, data);
	for (auto& c: prg) SAVELOAD_BYTE(stateMode, offset, data, c);
	SAVELOAD_BYTE(stateMode, offset, data, keyboardRow);
	if (stateMode== STATE_LOAD) sync();
	return offset;
}

uint16_t mapperNum =531;
} // namespace

MapperInfo MapperInfo_531 ={
	&mapperNum,
	_T("LittleCom PC-95"),
	COMPAT_FULL,
	load,
	reset,
	NULL,
	MMC3::cpuCycle,
	MMC3::ppuCycle,
	saveLoad,
	NULL,
	NULL
};
