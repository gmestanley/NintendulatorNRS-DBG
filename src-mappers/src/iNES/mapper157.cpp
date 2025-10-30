#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_FCG.h"
#include	"resource.h"

namespace {
EEPROM_I2C*	internalEPROM =NULL;
EEPROM_I2C*	externalEPROM =NULL;
uint8_t		internalEPROMData[256]; // The Datach-internal EEPROM is not denoted in the NES 2.0 header, hence not stored by the emulator in PRG-RAM address space.
uint8_t		externalEPROMLatch =0;

HWND		configWindow;
uint8_t		configCmd;
uint8_t		barcodeOut;
uint32_t	barcodeReadPos;
uint32_t	barcodeCycleCount;
TCHAR		barcode[256];
uint8_t		barcodeData[256];
int		numberOfNamedBarcodes;
struct NamedBarcode {
	TCHAR* name;
	TCHAR* code;
};
NamedBarcode*	namedBarcodes =NULL;

#include	"datach.hpp"

void	sync (void) {
	FCG::syncPRG(0x0F, 0x00);
	EMU->SetCHR_RAM8(0x00, 0);
	FCG::syncMirror();
}

int	MAPINT	read (int, int) {
	int result =0x10;
	if (internalEPROM) result &=internalEPROM->getData() *0x10;
	if (externalEPROM) result &=externalEPROM->getData() *0x10;
	result |=barcodeOut &0x08;
	result |=*EMU->OpenBus &~0x18;
	return result;
}

void	MAPINT	write (int bank, int addr, int val) {
	switch (addr &0xF) {
		case 0x0:	externalEPROMLatch &=~0x20;
				externalEPROMLatch |=val <<2 &0x20; // Put the external EEPROM's CLK bit into the same bit position as it was for register 0xD
				if (externalEPROM) externalEPROM->setPins(false, !!(externalEPROMLatch &0x20),     !!(externalEPROMLatch &0x40));
				break;
		case 0xD:	externalEPROMLatch &=~0x40;	// DAT is shared between the two EEPROMs
				externalEPROMLatch |=val &0x40;
				if (externalEPROM) externalEPROM->setPins(false, !!(externalEPROMLatch &0x20),     !!(externalEPROMLatch &0x40));
				if (internalEPROM) internalEPROM->setPins(false, val &0x80? true: !!(val &0x20), val &0x80? true: !!(val &0x40));
				break;
		default:	FCG::writeASIC(bank, addr, val);
				break;
	}
}

BOOL	MAPINT	load (void) {
	memset(internalEPROMData, 0, sizeof(internalEPROMData));
	ROM->ChipRAMData =internalEPROMData;
	ROM->ChipRAMSize =256;
	iNES_SetSRAM();
	internalEPROM =new EEPROM_24C02(0, internalEPROMData);
	if (ROM->INES_Version <2 || (ROM->INES2_PRGRAM &0xF0) >0)
		externalEPROM =new EEPROM_24C01(0, ROM->PRGRAMData);
	else
		externalEPROM =NULL;
	FCG::load(sync, FCGType::LZ93D50, NULL);
	configWindow =NULL;
	return TRUE;
}

void	MAPINT	reset (RESET_TYPE resetType) {
	if (resetType ==RESET_HARD) {
		switch(ROM->PRGROMCRC32) {
		case 0x19E81461: numberOfNamedBarcodes =36; namedBarcodes =cardsDragonBallZ;  break;
		case 0x5B457641: numberOfNamedBarcodes =39; namedBarcodes =cardsUltramanClub; break;
		case 0x0BE0A328: numberOfNamedBarcodes =77; namedBarcodes =cardsSDGundamWars; break;
		default:         numberOfNamedBarcodes = 1; namedBarcodes =cardsDefault;      break;
		}
		MapperInfo_157.Config(CFG_WINDOW, TRUE);
	}
	FCG::reset(resetType);
	for (int i =0x6; i<=0x7; i++) EMU->SetCPUReadHandler(i, read);
	for (int i =0x8; i<=0xF; i++) EMU->SetCPUWriteHandler(i, write);
	barcode[0] =0;
	barcodeData[0] =0xFF;
	barcodeReadPos =0;
	barcodeOut =0;
	barcodeCycleCount =0;
}

void	MAPINT	unload (void) {
	if (internalEPROM) {
		delete internalEPROM;
		internalEPROM =NULL;
	}
	if (externalEPROM) {
		delete externalEPROM;
		externalEPROM =NULL;
	}
	if (configWindow) 	{
		DestroyWindow(configWindow);
		configWindow =NULL;
	}
}

void	MAPINT	cpuCycle (void) {
	FCG::cpuCycle();
	if (++barcodeCycleCount >=1000) {
		barcodeCycleCount -=1000;
		if (barcodeData[barcodeReadPos] ==0xFF)
			barcodeOut =0;
		else 
			barcodeOut =(barcodeData[barcodeReadPos++] ^ 1) <<3;
	}
}

int	MAPINT	saveLoad (STATE_TYPE stateMode, int offset, unsigned char *data) {
	offset =FCG::saveLoad(stateMode, offset, data);
	
	for (int i =0; i <256; i++) SAVELOAD_BYTE(stateMode, offset, data, barcodeData[i]);
	SAVELOAD_LONG(stateMode, offset, data, barcodeReadPos);
	SAVELOAD_LONG(stateMode, offset, data, barcodeCycleCount);
	SAVELOAD_BYTE(stateMode, offset, data, barcodeOut);
	
	if (internalEPROM) offset =internalEPROM->saveLoad(stateMode, offset, data);
	if (internalEPROM) for (int i =0; i <256; i++) SAVELOAD_BYTE(stateMode, offset, data, internalEPROMData[i]);
	if (externalEPROM) SAVELOAD_BYTE(stateMode, offset, data, externalEPROMLatch);
	if (internalEPROM) offset =externalEPROM->saveLoad(stateMode, offset, data);
	return offset;
}

int	newBarcode (const TCHAR *rcode) {
	int prefix_parity_type[10][6] ={
		{ 0, 0, 0, 0, 0, 0 }, { 0, 0, 1, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1 }, { 0, 0, 1, 1, 1, 0 },
		{ 0, 1, 0, 0, 1, 1 }, { 0, 1, 1, 0, 0, 1 }, { 0, 1, 1, 1, 0, 0 }, { 0, 1, 0, 1, 0, 1 },
		{ 0, 1, 0, 1, 1, 0 }, { 0, 1, 1, 0, 1, 0 }
	};
	int data_left_odd[10][7] ={
		{ 0, 0, 0, 1, 1, 0, 1 }, { 0, 0, 1, 1, 0, 0, 1 }, { 0, 0, 1, 0, 0, 1, 1 }, { 0, 1, 1, 1, 1, 0, 1 },
		{ 0, 1, 0, 0, 0, 1, 1 }, { 0, 1, 1, 0, 0, 0, 1 }, { 0, 1, 0, 1, 1, 1, 1 }, { 0, 1, 1, 1, 0, 1, 1 },
		{ 0, 1, 1, 0, 1, 1, 1 }, { 0, 0, 0, 1, 0, 1, 1 }
	};
	int data_left_even[10][7] ={
		{ 0, 1, 0, 0, 1, 1, 1 }, { 0, 1, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1 }, { 0, 1, 0, 0, 0, 0, 1 },
		{ 0, 0, 1, 1, 1, 0, 1 }, { 0, 1, 1, 1, 0, 0, 1 }, { 0, 0, 0, 0, 1, 0, 1 }, { 0, 0, 1, 0, 0, 0, 1 },
		{ 0, 0, 0, 1, 0, 0, 1 }, { 0, 0, 1, 0, 1, 1, 1 }
	};
	int data_right[10][7] ={
		{ 1, 1, 1, 0, 0, 1, 0 }, { 1, 1, 0, 0, 1, 1, 0 }, { 1, 1, 0, 1, 1, 0, 0 }, { 1, 0, 0, 0, 0, 1, 0 },
		{ 1, 0, 1, 1, 1, 0, 0 }, { 1, 0, 0, 1, 1, 1, 0 }, { 1, 0, 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 1, 0, 0 },
		{ 1, 0, 0, 1, 0, 0, 0 }, { 1, 1, 1, 0, 1, 0, 0 }
	};
	uint8_t code[13 + 1];
	uint32_t tmp_p =0;
	int i, j;
	int len;

	for (i =len =0; i <13; i++) {
		if (!rcode[i]) break;
		if ((code[i] =rcode[i] - '0') > 9)
			return(0);
		len++;
	}
	if (len !=13 && len !=12 && len !=8 && len !=7) return(0);

	#define BS(x) barcodeData[tmp_p++] =x;
	for (j =0; j <32; j++) BS(0x00);

	/* Left guard bars */
	BS(1); BS(0); BS(1);
	if (len ==13 || len ==12) {
		uint32_t csum;

		for (i =0; i <6; i++)
			if (prefix_parity_type[code[0]][i]) {
				for (j =0; j <7; j++) {
					BS(data_left_even[code[i + 1]][j]);
				}
			} else
				for (j =0; j <7; j++) {
					BS(data_left_odd[code[i + 1]][j]);
				}

		/* Center guard bars */
		BS(0); BS(1); BS(0); BS(1); BS(0);

		for (i =7; i <12; i++)
			for (j =0; j <7; j++) {
				BS(data_right[code[i]][j]);
			}
		csum =0;
		for (i =0; i <12; i++) csum +=code[i] * ((i & 1) ? 3 : 1);
		csum =(10 - (csum % 10)) % 10;
		for (j =0; j <7; j++) {
			BS(data_right[csum][j]);
		}
	} else if (len ==8 || len ==7) {
		uint32_t csum =0;

		for (i =0; i <7; i++) csum +=(i & 1) ? code[i] : (code[i] * 3);

		csum =(10 - (csum % 10)) % 10;

		for (i =0; i <4; i++)
			for (j =0; j <7; j++) {
				BS(data_left_odd[code[i]][j]);
			}


		/* Center guard bars */
		BS(0); BS(1); BS(0); BS(1); BS(0);

		for (i =4; i <7; i++)
			for (j =0; j <7; j++) {
				BS(data_right[code[i]][j]);
			}

		for (j =0; j <7; j++) {
			BS(data_right[csum][j]);
		}
	}
	/* Right guard bars */
	BS(1); BS(0); BS(1);
	for (j =0; j <32; j++) {
		BS(0x00);
	}
	BS(0xFF);
	#undef BS
	barcodeReadPos =0;
	barcodeOut =0x8;
	barcodeCycleCount =0;
	return(1);
}

INT_PTR CALLBACK configProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	int selection;
	switch (message) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_BARCODE, WM_SETTEXT, 0, (LPARAM) barcode);
		selection =0;
		for (int i =0; i <numberOfNamedBarcodes; i++) {
			SendDlgItemMessage(hDlg, IDC_DATACH_CHARS, LB_ADDSTRING, 0, (LPARAM) namedBarcodes[i].name);
			if (_tcscmp(namedBarcodes[i].code, barcode) ==0) selection =i;
		}
		SendDlgItemMessage(hDlg, IDC_DATACH_CHARS, LB_SETCURSEL, selection, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BARCODE), (selection ==0)? TRUE: FALSE);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DATACH_CHARS:
			selection =SendDlgItemMessage(hDlg, IDC_DATACH_CHARS, LB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BARCODE), (selection ==0)? TRUE: FALSE);
			SendDlgItemMessage(hDlg, IDC_BARCODE, WM_SETTEXT, 0, (LPARAM) namedBarcodes[selection].code);
			if(HIWORD(wParam) ==LBN_DBLCLK) {
				configCmd =0x80;
				SendDlgItemMessage(hDlg, IDC_BARCODE, WM_GETTEXT, sizeof(barcode), (LPARAM) barcode);
			}
			return TRUE;
		case IDOK:
			configCmd =0x80;
			SendDlgItemMessage(hDlg, IDC_BARCODE, WM_GETTEXT, sizeof(barcode), (LPARAM) barcode);
			return TRUE;
		case IDCANCEL:
			configWindow =NULL;
			DestroyWindow(hDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

unsigned char	MAPINT	config (CFG_TYPE stateMode, unsigned char data) {
	switch (stateMode) {
	case CFG_WINDOW:
		if (data) {
			if (configWindow) break;
			configWindow =CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DATACH), hWnd, configProc);
			SetWindowPos(configWindow, hWnd, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		else	
			return TRUE;
		break;
	case CFG_QUERY:
		return configCmd;
		break;
	case CFG_CMD:
		if (data &0x80) newBarcode(barcode);
		configCmd =0;
		break;
	}
	return 0;
}

uint16_t mapperNum =157;
} // namespace

MapperInfo MapperInfo_157 ={
	&mapperNum,
	_T("Bandai Datach Joint ROM System"),
	COMPAT_FULL,
	load,
	reset,
	unload,
	cpuCycle,
	NULL,
	saveLoad,
	NULL,
	config
};