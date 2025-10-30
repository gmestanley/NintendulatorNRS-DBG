/*      Key groups in sets 1 and 2
	0       Unused

	1       Normal

	2       Right Alternate, Right Control, Numeric Keypad Enter
		E0 kk

	3       Navigational keys
		Normal or Shift+Num: E0 kk
		Shift case:          E0 !DIK_LSHIFT E0 kk
		Num case:            E0  DIK_LSHIFT E0 kk

	4       Numeric keypad ÷
		Normal:     E0 kk
		Shift case: E0 !DIK_LSHIFT E0 kk

	5       Print Screen/System Request
		Normal:     E0 DIK_LSHIFT E0 DIK_MULTIPLY
		Shift/Ctrl: E0 DIK_MULTIPLY
		Alt:        E0 54 (System Request)

	6       Pause: no break code
		Normal: E1 DIK_LCONTROL DIK_NUMLOCK E1 !DIK_LCONTROL !DIK_NUMLOCK
		Ctrl:   E0 DIK_SCROLL E0 !DIK_SCROLL
*/

const static struct {
	uint8_t	group;
	uint8_t codeSet1;  // Original 83-key PC/XT keyboard
	uint8_t codeSet2;  // Original 84-key PC/AT keyboard
	uint8_t codeSet3;  // Enhanced 101-/102-key keyboard
	uint8_t typematic; // Default setting for scan code set 3
} dik2scancode[256] ={
	{ 0                      }, // 0x00 Unused          IBM PS/2 Manual Key#
	{ 1, 0x01, 0x76, 0x08, 0 }, // 0x01 DIK_ESCAPE      110
	{ 1, 0x02, 0x16, 0x16, 1 }, // 0x02 DIK_1           2
	{ 1, 0x03, 0x1E, 0x1E, 1 }, // 0x03 DIK_2           3
	{ 1, 0x04, 0x26, 0x26, 1 }, // 0x04 DIK_3           4
	{ 1, 0x05, 0x25, 0x25, 1 }, // 0x05 DIK_4           5
	{ 1, 0x06, 0x2E, 0x2E, 1 }, // 0x06 DIK_5           6
	{ 1, 0x07, 0x36, 0x36, 1 }, // 0x07 DIK_6           7
	{ 1, 0x08, 0x3D, 0x3D, 1 }, // 0x08 DIK_7           8
	{ 1, 0x09, 0x3E, 0x3E, 1 }, // 0x09 DIK_8           9
	{ 1, 0x0A, 0x46, 0x46, 1 }, // 0x0A DIK_9           10
	{ 1, 0x0B, 0x45, 0x45, 1 }, // 0x0B DIK_0           11
	{ 1, 0x0C, 0x4E, 0x4E, 1 }, // 0x0C DIK_MINUS       12
	{ 1, 0x0D, 0x55, 0x55, 1 }, // 0x0D DIK_EQUALS      13
	{ 1, 0x0E, 0x66, 0x66, 1 }, // 0x0E DIK_BACK        15
	{ 1, 0x0F, 0x0D, 0x0D, 1 }, // 0x0F DIK_TAB         16
	{ 1, 0x10, 0x15, 0x15, 1 }, // 0x10 DIK_Q           17
	{ 1, 0x11, 0x1D, 0x1D, 1 }, // 0x11 DIK_W           18
	{ 1, 0x12, 0x24, 0x24, 1 }, // 0x12 DIK_E           19
	{ 1, 0x13, 0x2D, 0x2D, 1 }, // 0x13 DIK_R           20
	{ 1, 0x14, 0x2C, 0x2C, 1 }, // 0x14 DIK_T           21
	{ 1, 0x15, 0x35, 0x35, 1 }, // 0x15 DIK_Y           22
	{ 1, 0x16, 0x3C, 0x3C, 1 }, // 0x16 DIK_U           23
	{ 1, 0x17, 0x43, 0x43, 1 }, // 0x17 DIK_I           24
	{ 1, 0x18, 0x44, 0x44, 1 }, // 0x18 DIK_O           25
	{ 1, 0x19, 0x4D, 0x4D, 1 }, // 0x19 DIK_P           26
	{ 1, 0x1A, 0x54, 0x54, 1 }, // 0x1A DIK_LBRACKET    27
	{ 1, 0x1B, 0x5B, 0x5B, 1 }, // 0x1B DIK_RBRACKET    28
	{ 1, 0x1C, 0x5A, 0x5A, 1 }, // 0x1C DIK_RETURN      43
	{ 1, 0x1D, 0x14, 0x11, 0 }, // 0x1D DIK_LCONTROL    58
	{ 1, 0x1E, 0x1C, 0x1C, 1 }, // 0x1E DIK_A           31
	{ 1, 0x1F, 0x1B, 0x1B, 1 }, // 0x1F DIK_S           32
	{ 1, 0x20, 0x23, 0x23, 1 }, // 0x20 DIK_D           33
	{ 1, 0x21, 0x2B, 0x2B, 1 }, // 0x21 DIK_F           34
	{ 1, 0x22, 0x34, 0x34, 1 }, // 0x22 DIK_G           35
	{ 1, 0x23, 0x33, 0x33, 1 }, // 0x23 DIK_H           36
	{ 1, 0x24, 0x3B, 0x3B, 1 }, // 0x24 DIK_J           37
	{ 1, 0x25, 0x42, 0x42, 1 }, // 0x25 DIK_K           38
	{ 1, 0x26, 0x4B, 0x4B, 1 }, // 0x26 DIK_L           39
	{ 1, 0x27, 0x4C, 0x4C, 1 }, // 0x27 DIK_SEMICOLON   40
	{ 1, 0x28, 0x52, 0x52, 1 }, // 0x28 DIK_APOSTROPHE  41
	{ 1, 0x29, 0x0E, 0x0E, 1 }, // 0x29 DIK_GRAVE       1
	{ 1, 0x2A, 0x12, 0x12, 0 }, // 0x2A DIK_LSHIFT      44
	{ 1, 0x2B, 0x5D, 0x53, 1 }, // 0x2B DIK_BACKSLASH   29(US)/42(ISO)
	{ 1, 0x2C, 0x1A, 0x1A, 1 }, // 0x2C DIK_Z           46
	{ 1, 0x2D, 0x22, 0x22, 1 }, // 0x2D DIK_X           47
	{ 1, 0x2E, 0x21, 0x21, 1 }, // 0x2E DIK_C           48
	{ 1, 0x2F, 0x2A, 0x2A, 1 }, // 0x2F DIK_V           49
	{ 1, 0x30, 0x32, 0x32, 1 }, // 0x30 DIK_B           50
	{ 1, 0x31, 0x31, 0x31, 1 }, // 0x31 DIK_N           51
	{ 1, 0x32, 0x3A, 0x3A, 1 }, // 0x32 DIK_M           52
	{ 1, 0x33, 0x41, 0x41, 1 }, // 0x33 DIK_COMMA       53
	{ 1, 0x34, 0x49, 0x49, 1 }, // 0x34 DIK_PERIOD      54
	{ 1, 0x35, 0x4A, 0x4A, 1 }, // 0x35 DIK_SLASH       55
	{ 1, 0x36, 0x59, 0x59, 0 }, // 0x36 DIK_RSHIFT      57
	{ 1, 0x37, 0x7C, 0x7E, 0 }, // 0x37 DIK_MULTIPLY    100
	{ 1, 0x38, 0x11, 0x19, 0 }, // 0x38 DIK_LMENU       60
	{ 1, 0x39, 0x29, 0x29, 1 }, // 0x39 DIK_SPACE       61
	{ 1, 0x3A, 0x58, 0x14, 0 }, // 0x3A DIK_CAPITAL     30
	{ 1, 0x3B, 0x05, 0x07, 0 }, // 0x3B DIK_F1          112
	{ 1, 0x3C, 0x06, 0x0F, 0 }, // 0x3C DIK_F2          113
	{ 1, 0x3D, 0x04, 0x17, 0 }, // 0x3D DIK_F3          114
	{ 1, 0x3E, 0x0C, 0x1F, 0 }, // 0x3E DIK_F4          115
	{ 1, 0x3F, 0x03, 0x27, 0 }, // 0x3F DIK_F5          116
	{ 1, 0x40, 0x0B, 0x2F, 0 }, // 0x40 DIK_F6          117
	{ 1, 0x41, 0x83, 0x37, 0 }, // 0x41 DIK_F7          118
	{ 1, 0x42, 0x0A, 0x3F, 0 }, // 0x42 DIK_F8          119
	{ 1, 0x43, 0x01, 0x47, 0 }, // 0x43 DIK_F9          120
	{ 1, 0x44, 0x09, 0x4F, 0 }, // 0x44 DIK_F10         121
	{ 1, 0x45, 0x77, 0x76, 0 }, // 0x45 DIK_NUMLOCK     90
	{ 1, 0x46, 0x7E, 0x5F, 0 }, // 0x46 DIK_SCROLL      125
	{ 1, 0x47, 0x6C, 0x6C, 0 }, // 0x47 DIK_NUMPAD7     91
	{ 1, 0x48, 0x75, 0x75, 0 }, // 0x48 DIK_NUMPAD8     96
	{ 1, 0x49, 0x7D, 0x7D, 0 }, // 0x49 DIK_NUMPAD9     101
	{ 1, 0x4A, 0x7B, 0x84, 0 }, // 0x4A DIK_SUBTRACT    105
	{ 1, 0x4B, 0x6B, 0x6B, 0 }, // 0x4B DIK_NUMPAD4     92
	{ 1, 0x4C, 0x73, 0x73, 0 }, // 0x4C DIK_NUMPAD5     97
	{ 1, 0x4D, 0x74, 0x74, 0 }, // 0x4D DIK_NUMPAD6     102
	{ 1, 0x4E, 0x79, 0x7C, 1 }, // 0x4E DIK_ADD         106
	{ 1, 0x4F, 0x69, 0x69, 0 }, // 0x4F DIK_NUMPAD1     93
	{ 1, 0x50, 0x72, 0x72, 0 }, // 0x50 DIK_NUMPAD2     98
	{ 1, 0x51, 0x7A, 0x7A, 0 }, // 0x51 DIK_NUMPAD3     103
	{ 1, 0x52, 0x70, 0x70, 0 }, // 0x52 DIK_NUMPAD0     99
	{ 1, 0x53, 0x71, 0x71, 0 }, // 0x53 DIK_DECIMAL     104
	{ 0                      }, // 0x54 Unused
	{ 0                      }, // 0x55 Unused
	{ 1, 0x56, 0x61, 0x13, 1 }, // 0x56 <>|             45 (ISO)
	{ 0                      }, // 0x57
	{ 1, 0x57, 0x78, 0x56, 0 }, // 0x58 DIK_F11         122
	{ 1, 0x58, 0x07, 0x5E, 0 }, // 0x59 DIK_F12         123
	{ 0                      }, // 0x5A Unused
	{ 0                      }, // 0x5B Unused
	{ 0                      }, // 0x5C Unused
	{ 0                      }, // 0x5D Unused
	{ 0                      }, // 0x5E Unused
	{ 0                      }, // 0x5F Unused
	{ 0                      }, // 0x60 Unused
	{ 0                      }, // 0x61 Unused
	{ 0                      }, // 0x62 Unused
	{ 0                      }, // 0x63 Unused
	{ 0                      }, // 0x64 DIK_F13
	{ 0                      }, // 0x65 DIK_F14
	{ 0                      }, // 0x66 DIK_F15
	{ 0                      }, // 0x67 Unused
	{ 0                      }, // 0x68 Unused
	{ 0                      }, // 0x69 Unused
	{ 0                      }, // 0x6A Unused
	{ 0                      }, // 0x6B Unused
	{ 0                      }, // 0x6C Unused
	{ 0                      }, // 0x6D Unused
	{ 0                      }, // 0x6E Unused
	{ 0                      }, // 0x6F Unused
	{ 0                      }, // 0x70 DIK_KANA
	{ 0                      }, // 0x71 Unused
	{ 0                      }, // 0x72 Unused
	{ 0                      }, // 0x73 DIK_ABNT_C1
	{ 0                      }, // 0x74 Unused
	{ 0                      }, // 0x75 Unused
	{ 0                      }, // 0x76 Unused
	{ 0                      }, // 0x77 Unused
	{ 0                      }, // 0x78 Unused
	{ 0                      }, // 0x79 DIK_CONVERT
	{ 0                      }, // 0x7A Unused
	{ 0                      }, // 0x7B DIK_NOCONVERT
	{ 0                      }, // 0x7C Unused
	{ 0                      }, // 0x7D DIK_YEN
	{ 0                      }, // 0x7E DIK_ABNT_C2
	{ 0                      }, // 0x7F Unused
	{ 0                      }, // 0x80 Unused
	{ 0                      }, // 0x81 Unused
	{ 0                      }, // 0x82 Unused
	{ 0                      }, // 0x83 Unused
	{ 0                      }, // 0x84 Unused
	{ 0                      }, // 0x85 Unused
	{ 0                      }, // 0x86 Unused
	{ 0                      }, // 0x87 Unused
	{ 0                      }, // 0x88 Unused
	{ 0                      }, // 0x89 Unused
	{ 0                      }, // 0x8A Unused
	{ 0                      }, // 0x8B Unused
	{ 0                      }, // 0x8C Unused
	{ 0                      }, // 0x8D DIK_NUMPADEQUALS
	{ 0                      }, // 0x8E Unused
	{ 0                      }, // 0x8F Unused
	{ 0                      }, // 0x90 DIK_PREVTRACK
	{ 0                      }, // 0x91 DIK_AT
	{ 0                      }, // 0x92 DIK_COLON
	{ 0                      }, // 0x93 DIK_UNDERLINE
	{ 0                      }, // 0x94 DIK_KANJI
	{ 0                      }, // 0x95 DIK_STOP
	{ 0                      }, // 0x96 DIK_AX
	{ 0                      }, // 0x97 DIK_UNLABELED
	{ 0                      }, // 0x98 Unused
	{ 0                      }, // 0x99 DIK_NEXTTRACK
	{ 0                      }, // 0x9A Unused
	{ 0                      }, // 0x9B Unused
	{ 2, 0x1C, 0x5A, 0x79, 0 }, // 0x9C DIK_NUMPADENTER 108
	{ 2, 0x1D, 0x14, 0x58, 0 }, // 0x9D DIK_RCONTROL    64
	{ 0                      }, // 0x9E Unused
	{ 0                      }, // 0x9F Unused
	{ 0                      }, // 0xA0 DIK_MUTE
	{ 0                      }, // 0xA1 DIK_CALCULATOR
	{ 0                      }, // 0xA2 DIK_PLAYPAUSE
	{ 0                      }, // 0xA3 Unused
	{ 0                      }, // 0xA4 DIK_MEDIASTOP
	{ 0                      }, // 0xA5 Unused
	{ 0                      }, // 0xA6 Unused
	{ 0                      }, // 0xA7 Unused
	{ 0                      }, // 0xA8 Unused
	{ 0                      }, // 0xA9 Unused
	{ 0                      }, // 0xAA Unused
	{ 0                      }, // 0xAB Unused
	{ 0                      }, // 0xAC Unused
	{ 0                      }, // 0xAD Unused
	{ 0                      }, // 0xAE DIK_VOLUMEDOWN
	{ 0                      }, // 0xAF Unused
	{ 0                      }, // 0xB0 DIK_VOLUMEUP
	{ 0                      }, // 0xB1 Unused
	{ 0                      }, // 0xB2 DIK_WEBHOME
	{ 0                      }, // 0xB3 DIK_NUMPADCOMMA 104
	{ 0                      }, // 0xB4 Unused
	{ 4, 0x35, 0x4A, 0x77, 0 }, // 0xB5 DIK_DIVIDE      95
	{ 0                      }, // 0xB6 Unused
	{ 5, 0x2A, 0x7C, 0x57, 0 }, // 0xB7 DIK_SYSRQ       124
	{ 2, 0x38, 0x11, 0x39, 0 }, // 0xB8 DIK_RMENU       62
	{ 0                      }, // 0xB9 Unused
	{ 0                      }, // 0xBA Unused
	{ 0                      }, // 0xBB Unused
	{ 0                      }, // 0xBC Unused
	{ 0                      }, // 0xBD Unused
	{ 0                      }, // 0xBE Unused
	{ 0                      }, // 0xBF Unused
	{ 0                      }, // 0xC0 Unused
	{ 0                      }, // 0xC1 Unused
	{ 0                      }, // 0xC2 Unused
	{ 0                      }, // 0xC3 Unused
	{ 0                      }, // 0xC4 Unused
	{ 6, 0x45, 0x77, 0x62, 0 }, // 0xC5 DIK_PAUSE       126
	{ 0                      }, // 0xC6 Unused
	{ 3, 0x47, 0x6C, 0x6E, 0 }, // 0xC7 DIK_HOME        80
	{ 3, 0x48, 0x75, 0x63, 1 }, // 0xC8 DIK_UP          83
	{ 3, 0x49, 0x7D, 0x6F, 0 }, // 0xC9 DIK_PRIOR       85
	{ 0                      }, // 0xCA Unused
	{ 3, 0x4B, 0x6B, 0x61, 1 }, // 0xCB DIK_LEFT        79
	{ 0                      }, // 0xCC Unused
	{ 3, 0x4D, 0x74, 0x6A, 1 }, // 0xCD DIK_RIGHT       89
	{ 0                      }, // 0xCE Unused
	{ 3, 0x4F, 0x69, 0x65, 0 }, // 0xCF DIK_END         81
	{ 3, 0x58, 0x72, 0x60, 1 }, // 0xD0 DIK_DOWN        84
	{ 3, 0x51, 0x7A, 0x6D, 0 }, // 0xD1 DIK_NEXT        86
	{ 3, 0x52, 0x70, 0x67, 0 }, // 0xD2 DIK_INSERT      75
	{ 3, 0x53, 0x71, 0x64, 1 }, // 0xD3 DIK_DELETE      76
	{ 0                      }, // 0xD4 Unused
	{ 0                      }, // 0xD5 Unused
	{ 0                      }, // 0xD6 Unused
	{ 0                      }, // 0xD7 Unused
	{ 0                      }, // 0xD8 Unused
	{ 0                      }, // 0xD9 Unused
	{ 0                      }, // 0xDA Unused
	{ 0                      }, // 0xDB DIK_LWIN
	{ 0                      }, // 0xDC DIK_RWIN
	{ 0                      }, // 0xDD DIK_APPS
	{ 0                      }, // 0xDE DIK_POWER
	{ 0                      }, // 0xDF DIK_SLEEP
	{ 0                      }, // 0xE0 Unused
	{ 0                      }, // 0xE1 Unused
	{ 0                      }, // 0xE2 Unused
	{ 0                      }, // 0xE3 DIK_WAKE
	{ 0                      }, // 0xE4 Unused
	{ 0                      }, // 0xE5 DIK_WEBSEARCH
	{ 0                      }, // 0xE6 DIK_WEBFAVORITES
	{ 0                      }, // 0xE7 DIK_WEBREFRESH
	{ 0                      }, // 0xE8 DIK_WEBSTOP
	{ 0                      }, // 0xE9 DIK_WEBFORWARD
	{ 0                      }, // 0xEA DIK_WEBBACK
	{ 0                      }, // 0xEB DIK_MYCOMPUTER
	{ 0                      }, // 0xEC DIK_MAIL
	{ 0                      }, // 0xED DIK_MEDIASELECT
	{ 0                      }, // 0xEE Unused
	{ 0                      }, // 0xEF Unused
	{ 0                      }, // 0xF0 Unused
	{ 0                      }, // 0xF1 Unused
	{ 0                      }, // 0xF2 Unused
	{ 0                      }, // 0xF3 Unused
	{ 0                      }, // 0xF4 Unused
	{ 0                      }, // 0xF5 Unused
	{ 0                      }, // 0xF6 Unused
	{ 0                      }, // 0xF7 Unused
	{ 0                      }, // 0xF8 Unused
	{ 0                      }, // 0xF9 Unused
	{ 0                      }, // 0xFA Unused
	{ 0                      }, // 0xFB Unused
	{ 0                      }, // 0xFC Unused
	{ 0                      }, // 0xFD Unused
	{ 0                      }, // 0xFE Unused
	{ 0                      }  // 0xFF Unused
};

class PS2Keyboard {
	std::queue<uint8_t>& dataFromHost;
	std::queue<uint8_t>& dataToHost;
	const uint16_t id;
public:
	PS2Keyboard (std::queue<uint8_t>&, std::queue<uint8_t>&, uint16_t);
	run();
};

PS2Keyboard::run() {
	if (batCount && !--batCount) {
		setDefaults();
		dataToHost.push(0xAA);
	}
	if (scanCount && !--scanCount && scanEnabled && dataFromHost.empty()) {
	}
	if (!dataFromHost.empty()) {
		if (!commandNumber) commandNumber =dataFromHost.front();
		switch(commandNumber) {
			case 0xED: // Set/Reset Status Indicators
				break;
			case 0xEE: // Echo
				dataToHost.push(0xEE);
				command =0;
				break;
			case 0xEF: // Invalid command
				dataToHost.push(0xFE);
				command =0;
				break;
			case 0xF0: // Set Alternate Scan Codes
				if (dataFromHost.size() <2)
					dataToHost.push(0xFA);
				else {
					dataToHost.push(0xFA);
					dataFromHost.pop();
					switch(dataFromHost.front()) {
						case 0:
							dataToHost.push(scanCodeSet);
						case 1: case 2: case 3:
							scanCodeSet =dataFromHost.front();
					}
					command =0;
				}
				break;
			case 0xF1: // Invalid command
				dataToHost.push(0xFE);
				command =0;
				break;
			case 0xF2: // Read ID
				dataToHost.push(0xFA);
				dataToHost.push(id &0xFF);
				dataToHost.push(id >>8 &0xFF);
				command =0;
				break;
			case 0xF3: // Set Typematic Rate/Delay
				break;
			case 0xF4: // Enable
				dataToHost.push(0xFA);
				clearBuffer();
				scanEnabled =true;
				command =0;
				break;
			case 0xF5: // Default Disable
				dataToHost.push(0xFA);
				setDefaults();
				scanEnabled =false;
				command =0;
				break;
			case 0xF6: // Set Default
				dataToHost.push(0xFA);
				setDefaults();
				command =0;
				break;
			case 0xF7: case 0xF8: case 0xF9: case 0xFA: // Set All Keys
				dataToHost.push(0xFA);
				clearBuffer();
				for (auto& key: typematicSetting) key =commandNumber -0xF7;
				command =0;
				break;
			case 0xFB: case 0xFC: case 0xFD: // Set Key Type
				if (dataFromHost.size() <2)
					clearBuffer();
				else {
					dataFromHost.pop();
					dataToHost.push(0xFA);
					typematicSetting[dataFromHost.front()] =commandNumber -0xFB;
					command =0;
				}
				break;
			case 0xFE: // Resend
				break;
			case 0xFF: // Reset
				dataToHost.push(0xFA);
				clearBuffer();
				scanEnabled =false;
				batCount =300 *CYCLES_IN_MILLISECOND
				command =0;
				break;
		}
		dataFromHost.pop();
	}
}
