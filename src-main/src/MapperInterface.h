#pragma once
#include <vector>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

/* Mapper Interface version (3.9) */

#ifdef	UNICODE
#define	CurrentMapperInterface	0x84030009
#else	/* !UNICODE */
#define	CurrentMapperInterface	0x04030009
#endif	/* UNICODE */

/* Function types */

#define	MAPINT	__cdecl

typedef	void	(MAPINT *FCPUWrite)	(int Bank,int Addr,int Val);
typedef	int	(MAPINT *FCPURead)	(int Bank,int Addr);

typedef	void	(MAPINT *FPPUWrite)	(int Bank,int Addr,int Val);
typedef	int	(MAPINT *FPPURead)	(int Bank,int Addr);

/* Mapper Interface Structure - Pointers to data and functions within emulator */
enum RESET_TYPE	{ RESET_NONE, RESET_SOFT, RESET_HARD };

#define CONSOLE_HVC		0x00
#define CONSOLE_VS		0x01
#define CONSOLE_PC10		0x02
#define CONSOLE_BITCORP		0x03
#define CONSOLE_EPSM		0x04
#define CONSOLE_VT01STN		0x05
#define CONSOLE_VT02		0x06
#define CONSOLE_VT03		0x07
#define CONSOLE_VT09		0x08
#define CONSOLE_VT32		0x09
#define CONSOLE_VT369		0x0A
#define CONSOLE_UM6578		0x0B
#define CONSOLE_FC_NETWORK	0x0C

#define VS_NORMAL	0x00
#define VS_RBI		0x01
#define VS_TKO		0x02
#define VS_SUPERXEVIOUS	0x03
#define VS_ICECLIMBERJ	0x04
#define VS_DUAL		0x05
#define VS_BUNGELING	0x06

#define INPUT_UNSPECIFIED	   0x00
#define INPUT_STANDARD		   0x01
#define INPUT_FOURSCORE		   0x02
#define INPUT_FAMI4PLAY		   0x03
#define INPUT_VS_NORMAL		   0x04
#define INPUT_VS_REVERSED	   0x05
#define INPUT_VS_PINBALL	   0x06
#define INPUT_VS_ZAPPER		   0x07
#define INPUT_ZAPPER_2P		   0x08
#define INPUT_TWO_ZAPPERS	   0x09
#define INPUT_BANDAI_HYPER_SHOT	   0x0A
#define INPUT_POWER_PAD_A	   0x0B
#define INPUT_POWER_PAD_B	   0x0C
#define INPUT_FAMILY_TRAINER_A	   0x0D
#define INPUT_FAMILY_TRAINER_B	   0x0E
#define INPUT_VAUS_NES		   0x0F
#define INPUT_VAUS_FAMICOM	   0x10
#define INPUT_TWO_VAUS		   0x11
#define INPUT_KONAMI_HYPER_SHOT	   0x12
#define INPUT_PACHINKO		   0x13
#define INPUT_BOXING		   0x14
#define INPUT_JISSEN_MAHJONG	   0x15
#define INPUT_PARTY_TAP		   0x16
#define INPUT_OEKA_KIDS_TABLET	   0x17
#define INPUT_SUNSOFT_BARCODE	   0x18
#define INPUT_MIRACLE_PIANO	   0x19
#define INPUT_POKKUN_MOGURAA	   0x1A
#define INPUT_TOP_RIDER		   0x1B
#define INPUT_DOUBLE_FISTED	   0x1C
#define INPUT_FAMICOM_3D	   0x1D
#define INPUT_DOREMIKKO		   0x1E
#define INPUT_GYROMITE		   0x1F
#define INPUT_DATA_RECORDER	   0x20
#define INPUT_TURBO_FILE	   0x21
#define INPUT_BATTLE_BOX	   0x22
#define INPUT_FAMILY_BASIC	   0x23
#define INPUT_PEC_586		   0x24
#define INPUT_BIT_79		   0x25
#define INPUT_SUBOR_KEYBOARD	   0x26
#define INPUT_BELSONIC_MOUSE       0x27
#define INPUT_SUBOR_MOUSE_1P       0x28
#define INPUT_SNES_MOUSE_1P    	   0x29
#define INPUT_GENERIC_MULTICART	   0x2A
#define INPUT_SNES_CONTROLLER 	   0x2B
#define INPUT_RACERMATE		   0x2C
#define INPUT_UFORCE		   0x2D
#define INPUT_STACKUP		   0x2E
#define INPUT_CITY_PATROLMAN	   0x2F
#define INPUT_SHARP_C1_CASSETTE	   0x30
#define INPUT_STANDARD_INVERTED	   0x31
#define INPUT_SUDOKU_EXCALIBUR 	   0x32
#define INPUT_ABL_PINBALL      	   0x33
#define INPUT_GOLDEN_NUGGET_CASINO 0x34
#define INPUT_KEDA_KEYBOARD        0x35
#define INPUT_SUBOR_MOUSE_2P       0x36
#define INPUT_PORT_TEST_CONTROLLER 0x37
#define INPUT_BANDAI_GAMEPAD       0x38
#define INPUT_VENOM_DANCE_MAT      0x39
#define INPUT_LG_TV_REMOTE_CONTROL 0x3A
#define INPUT_FAMICOM_NETWORK_CTRL 0x3B
#define INPUT_KING_FISHING         0x3C
#define INPUT_CROAKY_KARAOKE       0x3D
#define INPUT_KINGWON_KEYBOARD     0x3E
#define INPUT_ZECHENG_KEYBOARD     0x3F
#define INPUT_SUBOR_PS2MOUSE_2P    0x40
#define INPUT_UM6578_KBD_MOUSE     0x41
#define INPUT_UM6578_PS2_MOUSE     0x42
#define INPUT_YUXING_MOUSE_1P      0x43
#define INPUT_SUBOR_YUXING_1P      0x44
#define INPUT_TV_PUMP              0x45
#define INPUT_BBK_KBD_MOUSE        0x46
#define INPUT_MAGICAL_COOKING      0x47
#define INPUT_SNES_MOUSE_2P    	   0x48
#define INPUT_ZAPPER_1P		   0x49
#define INPUT_VAUS_PROTOTYPE       0x4A
#define INPUT_TRIFACE_MAHJONG      0x4B
#define INPUT_MAHJONG_GEKITOU      0x4C
#define INPUT_SUBOR_PS2INV_2P      0x4D
#define INPUT_XT_KEYBOARD          0x4E

struct	EmulatorInterface
{
/* Functions for managing read/write handlers */
	void		(MAPINT *SetCPUReadHandler)	(int,FCPURead);
	FCPURead	(MAPINT *GetCPUReadHandler)	(int);

	void		(MAPINT *SetCPUReadHandlerDebug)(int,FCPURead);
	FCPURead	(MAPINT *GetCPUReadHandlerDebug)(int);

	void		(MAPINT *SetCPUWriteHandler)	(int,FCPUWrite);
	FCPUWrite	(MAPINT *GetCPUWriteHandler)	(int);

	void		(MAPINT *SetPPUReadHandler)	(int,FPPURead);
	FPPURead	(MAPINT *GetPPUReadHandler)	(int);

	void		(MAPINT *SetPPUReadHandlerDebug)(int,FPPURead);
	FPPURead	(MAPINT *GetPPUReadHandlerDebug)(int);

	void		(MAPINT *SetPPUWriteHandler)	(int,FPPUWrite);
	FPPUWrite	(MAPINT *GetPPUWriteHandler)	(int);

/* Functions for mapping PRG */
	void		(MAPINT *SetPRGROMSize)		(int);
	void		(MAPINT *SetPRGRAMSize)		(int);
	void		(MAPINT *SetPRG_ROM4)		(int,int);
	void		(MAPINT *SetPRG_ROM8)		(int,int);
	void		(MAPINT *SetPRG_ROM16)		(int,int);
	void		(MAPINT *SetPRG_ROM32)		(int,int);
	int		(MAPINT *GetPRG_ROM4)		(int);		/* -1 if no ROM mapped */

	void		(MAPINT *SetPRG_RAM4)		(int,int);
	void		(MAPINT *SetPRG_RAM8)		(int,int);
	void		(MAPINT *SetPRG_RAM16)		(int,int);
	void		(MAPINT *SetPRG_RAM32)		(int,int);
	int		(MAPINT *GetPRG_RAM4)		(int);		/* -1 if no RAM mapped */

	unsigned char *	(MAPINT *GetPRG_Ptr4)		(int);
	void		(MAPINT *SetPRG_Ptr4)		(int,unsigned char *,BOOL);
	void		(MAPINT *SetPRG_OB4)		(int);		/* Open bus */

/* Functions for mapping CHR */
	void		(MAPINT *SetCHRROMSize)		(int);
	void		(MAPINT *SetCHRRAMSize)		(int);
	void		(MAPINT *SetCHR_ROM1)		(int,int);
	void		(MAPINT *SetCHR_ROM2)		(int,int);
	void		(MAPINT *SetCHR_ROM4)		(int,int);
	void		(MAPINT *SetCHR_ROM8)		(int,int);
	int		(MAPINT *GetCHR_ROM1)		(int);		/* -1 if no ROM mapped */

	void		(MAPINT *SetCHR_RAM1)		(int,int);
	void		(MAPINT *SetCHR_RAM2)		(int,int);
	void		(MAPINT *SetCHR_RAM4)		(int,int);
	void		(MAPINT *SetCHR_RAM8)		(int,int);
	int		(MAPINT *GetCHR_RAM1)		(int);		/* -1 if no RAM mapped */

	void		(MAPINT *SetCHR_NT1)		(int,int);
	int		(MAPINT *GetCHR_NT1)		(int);		/* -1 if no nametable mapped */

	unsigned char *	(MAPINT *GetCHR_Ptr1)		(int);
	void		(MAPINT *SetCHR_Ptr1)		(int,unsigned char *,BOOL);
	void		(MAPINT *SetCHR_OB1)		(int);		/* Open bus */

/* Functions for controlling mirroring */
	void		(MAPINT *Mirror_H)		(void);
	void		(MAPINT *Mirror_V)		(void);
	void		(MAPINT *Mirror_4)		(void);
	void		(MAPINT *Mirror_S0)		(void);
	void		(MAPINT *Mirror_S1)		(void);
	void		(MAPINT *Mirror_Custom)	(int,int,int,int);

/* IRQ */
	void		(MAPINT *SetIRQ)		(int);		/* Sets the state of the /IRQ line */
	void		(MAPINT *SetIRQ2)		(int, int);	/* Sets the state of the /IRQ line on selected CPU */

/* Save RAM Handling */
	void		(MAPINT *Set_SRAMSize)		(int);		/* Sets the size of the SRAM (in bytes) */

/* Misc Callbacks */
	void		(MAPINT *DbgOut)		(const TCHAR *,...);	/* Echo text to debug window */
	void		(MAPINT *StatusOut)		(const TCHAR *,...);	/* Echo text on status bar */
	void		(MAPINT *SetPC)			(int);		/* Set CPU Program Counter */
	void		(MAPINT *ForceReset)		(RESET_TYPE);
	void		(MAPINT *RegisterWindow)	(HWND);
	void		(MAPINT *UnregisterWindow)	(HWND);
	void		(MAPINT *StopCPU)		(void);
	void		(*tapeOut)                      (bool);
	bool		(*tapeIn)                       ();
	
	/* Data fields */
	unsigned char *	OpenBus;			/* pointer to last value on the CPU data bus */
	BOOL		*BootlegExpansionAudio;
	BOOL		*CleanN163;
	uint16_t	*cpuPC;
	bool		*cpuFI;
	BYTE		*keyState;
	DIMOUSESTATE2	*mouseState;
	
	/* Multicart fields */
	BOOL		*multiCanSave;
	uint8_t*	*multiPRGStart;
	uint8_t*	*multiCHRStart;
	size_t		*multiPRGSize;
	size_t		*multiCHRSize;
	uint16_t	*multiMapper;
	uint8_t		*multiSubmapper;
	uint8_t		*multiMirroring;
	
	/* Default handlers */
	FCPURead	ReadPRG;
	FCPURead	ReadPRGDebug;
	FCPUWrite	WritePRG;
	FPPURead	ReadCHR;
	FPPURead	ReadCHRDebug;
	FPPUWrite	WriteCHR;
	FCPURead	ReadAPU;
	FCPURead	ReadAPUDebug;
	FCPUWrite	WriteAPU;
};

enum COMPAT_TYPE	{ COMPAT_NONE, COMPAT_PARTIAL, COMPAT_NEARLY, COMPAT_FULL, COMPAT_NUMTYPES };

/* Mapper Information structure - Contains pointers to mapper functions, sent to emulator on load mapper  */


enum STATE_TYPE	{ STATE_SIZE, STATE_SAVE, STATE_LOAD };

enum CFG_TYPE	{ CFG_WINDOW, CFG_QUERY, CFG_CMD, CFG_HOTPLUG };

struct	MapperInfo
{
/* Mapper Information */
	void *		MapperId;
	TCHAR *		Description;
	COMPAT_TYPE	Compatibility;

/* Mapper Functions */
	BOOL		(MAPINT *Load)		(void);
	void		(MAPINT *Reset)	(RESET_TYPE);		/* ResetType */
	void		(MAPINT *Unload)	(void);
	void		(MAPINT *CPUCycle)	(void);
	void		(MAPINT *PPUCycle)	(int,int,int,int);	/* Address, Scanline, Cycle, IsRendering */
	int		(MAPINT *SaveLoad)	(STATE_TYPE,int,unsigned char *);	/* Mode, Offset, Data */
	int		(MAPINT *GenSound)	(int);			/* Cycles */
	unsigned char	(MAPINT *Config)	(CFG_TYPE,unsigned char);	/* Mode, Data */
};

/* ROM Information Structure - Contains information about the ROM currently loaded */

enum ROM_TYPE	{ ROM_UNDEFINED, ROM_INES, ROM_UNIF, ROM_FDS, ROM_NSF, ROM_NUMTYPES };

struct	ROMInfo
{
	TCHAR *		Filename;
	ROM_TYPE	ROMType;
	union
	{
		struct
		{
			WORD	INES_MapperNum;
			BYTE	INES_Flags;		/* byte 6 flags in lower 4 bits, byte 7 flags in upper 4 bits */
			WORD	INES_PRGSize;		/* number of 16KB banks */
			WORD	INES_CHRSize;		/* number of 8KB banks */
			BYTE	INES_Version;		/* 1 for standard .NES, 2 Denotes presence of NES 2.0 data */
			BYTE	INES2_SubMapper;	/* NES 2.0 - submapper */
			BYTE	INES2_TVMode;		/* NES 2.0 - NTSC/PAL indicator */
			BYTE	INES2_PRGRAM;		/* NES 2.0 - PRG RAM counts, batteried and otherwise */
			BYTE	INES2_CHRRAM;		/* NES 2.0 - CHR RAM counts, batteried and otherwise */
			BYTE	INES2_VSPalette;
			BYTE	INES2_VSFlags;
		};	/* INES */
		struct
		{
			char *	UNIF_BoardName;
			BYTE	UNIF_Mirroring;
			BYTE	UNIF_NTSCPAL;		/* 0 for NTSC, 1 for PAL, 2 for either */
			BOOL	UNIF_Battery;
			BYTE	UNIF_NumPRG;		/* number of PRG# blocks */
			BYTE	UNIF_NumCHR;		/* number of CHR# blocks */
			DWORD	UNIF_PRGSize[16];	/* size of each PRG block, in bytes */
			DWORD	UNIF_CHRSize[16];	/* size of each CHR block, in bytes */
		};	/* UNIF */
		struct
		{
			BYTE	FDS_NumSides;
			BYTE	FDS_Type;
			BYTE	FDS_SubType;
			BYTE	FDS_Flags;
		};	/* FDS */
		struct
		{
			BYTE	NSF_NumSongs;
			BYTE	NSF_InitSong;
			WORD	NSF_InitAddr;
			WORD	NSF_PlayAddr;
			char *	NSF_Title;
			char *	NSF_Artist;
			char *	NSF_Copyright;
			WORD	NSF_NTSCSpeed;		/* Number of microseconds between PLAY calls for NTSC */
			WORD	NSF_PALSpeed;		/* Number of microseconds between PLAY calls for PAL */
			BYTE	NSF_NTSCPAL;		/* 0 for NTSC, 1 for PAL, 2 for either */
			BYTE	NSF_SoundChips;
			BYTE	NSF_InitBanks[8];
			DWORD	NSF_DataSize;		/* total amount of (PRG) data present */
		};	/* NSF */
		struct
		{
			BYTE	reserved[256];
		};	/* reserved for additional file types */
	};
	uint8_t		*PRGROMData,
			*PRGRAMData,
			*CHRROMData,
			*CHRRAMData,
			*MiscROMData,
			*ChipRAMData;
	size_t		PRGROMSize,
			PRGRAMSize,
			CHRROMSize,
			CHRRAMSize,
			MiscROMSize,
			ChipRAMSize;
	uint32_t	PRGROMCRC32,
			CHRROMCRC32,
			OverallCRC32;
	uint8_t		ConsoleType,
			InputType;
	
	uint32_t	dipValue;
	BOOL		dipValueSet;
	BOOL		ReloadUponHardReset;
	
std::vector<uint8_t>	*diskData28;
std::vector<uint8_t>	*diskData35;
	bool		changedDisk28;
	bool		modifiedSide28;
	bool		changedDisk35;
	bool		modifiedDisk35;
	
	uint32_t	vt369relative;
	uint32_t	vt369bgData;
	uint32_t	vt369sprData;
};

/* DLL Information Structure - Contains general information about the mapper DLL */

struct	DLLInfo
{
	TCHAR *		Description;
	int		Date;
	int		Version;
	MapperInfo *(MAPINT *LoadMapper)	(const ROMInfo *);
	void		(MAPINT *UnloadMapper)	(void);
};

typedef	DLLInfo *(MAPINT *FLoadMapperDLL)	(HWND, const EmulatorInterface *, int);
typedef	void	(MAPINT *FUnloadMapperDLL)	(void);

extern	EmulatorInterface	EI;
extern	ROMInfo		RI;
extern	DLLInfo		*DI;
extern	MapperInfo	*MI, *MI2;

namespace MapperInterface
{
extern const TCHAR *CompatLevel[COMPAT_NUMTYPES];

void	Init (void);
void	Destroy (void);
BOOL	LoadMapper (const ROMInfo *ROM);
void	UnloadMapper (void);
} // namespace MapperInterface

#define VSDUAL (RI.ConsoleType ==CONSOLE_VS && (RI.INES2_VSFlags ==VS_DUAL || RI.INES2_VSFlags ==VS_BUNGELING))

typedef	union
{
	struct
	{
		unsigned n0 : 4;
		unsigned n1 : 4;
		unsigned n2 : 4;
		unsigned n3 : 4;
	};
	uint8_t b[2];
	uint16_t s[1];
}	uint16_n;

typedef	union
{
	struct
	{
		unsigned n0 : 4;
		unsigned n1 : 4;
		unsigned n2 : 4;
		unsigned n3 : 4;
		unsigned n4 : 4;
		unsigned n5 : 4;
		unsigned n6 : 4;
		unsigned n7 : 4;
	};
	uint8_t b[4];
	uint16_t s[2];
	uint32_t l[1];
}	uint32_n;
typedef	union
{
	struct
	{
		unsigned n0 : 4;
		unsigned n1 : 4;
		unsigned n2 : 4;
		unsigned n3 : 4;
		unsigned n4 : 4;
		unsigned n5 : 4;
		unsigned n6 : 4;
		unsigned n7 : 4;
		unsigned n8 : 4;
		unsigned n9 : 4;
		unsigned nA : 4;
		unsigned nB : 4;
		unsigned nC : 4;
		unsigned nD : 4;
		unsigned nE : 4;
		unsigned nF : 4;
	};
	uint8_t b[8];
	uint16_t s[4];
	uint32_t l[2];
	uint64_t ll[1];
}	uint64_n;
#define b0 b[0]
#define b1 b[1]
#define b2 b[2]
#define b3 b[3]
#define b4 b[4]
#define b5 b[5]
#define b6 b[6]
#define b7 b[7]
#define s0 s[0]
#define s1 s[1]
#define l0 l[0]
#define l1 l[1]
#define ll0 ll[0]

#define	SAVELOAD_BYTE(mode,offset,data,value) \
do { \
	if (mode == STATE_SAVE) data[offset++] = value; \
	else if (mode == STATE_LOAD) value = data[offset++]; \
	else if (mode == STATE_SIZE) offset++; \
} while (0)
#define	SAVELOAD_WORD(mode,offset,data,value) \
do { \
	uint16_n sl_tmp; \
	if (mode == STATE_SAVE) { sl_tmp.s0 = value; data[offset++] = sl_tmp.b0; data[offset++] = sl_tmp.b1; } \
	else if (mode == STATE_LOAD) { sl_tmp.b0 = data[offset++]; sl_tmp.b1 = data[offset++]; value = sl_tmp.s0; } \
	else if (mode == STATE_SIZE) offset += 2; \
} while (0)
#define	SAVELOAD_LONG(mode,offset,data,value) \
do { \
	uint32_n sl_tmp; \
	if (mode == STATE_SAVE) { sl_tmp.l0 = value; data[offset++] = sl_tmp.b0; data[offset++] = sl_tmp.b1; data[offset++] = sl_tmp.b2; data[offset++] = sl_tmp.b3; } \
	else if (mode == STATE_LOAD) { sl_tmp.b0 = data[offset++]; sl_tmp.b1 = data[offset++]; sl_tmp.b2 = data[offset++]; sl_tmp.b3 = data[offset++]; value = sl_tmp.l0; } \
	else if (mode == STATE_SIZE) offset += 4; \
} while (0)
#define	SAVELOAD_64(mode,offset,data,value) \
do { \
	uint64_n sl_tmp; \
	if (mode == STATE_SAVE) { sl_tmp.ll0 = value; data[offset++] = sl_tmp.b0; data[offset++] = sl_tmp.b1; data[offset++] = sl_tmp.b2; data[offset++] = sl_tmp.b3; data[offset++] = sl_tmp.b4; data[offset++] = sl_tmp.b5; data[offset++] = sl_tmp.b6; data[offset++] = sl_tmp.b7; } \
	else if (mode == STATE_LOAD) { sl_tmp.b0 = data[offset++]; sl_tmp.b1 = data[offset++]; sl_tmp.b2 = data[offset++]; sl_tmp.b3 = data[offset++]; sl_tmp.b4 = data[offset++]; sl_tmp.b5 = data[offset++]; sl_tmp.b6 = data[offset++]; sl_tmp.b7 = data[offset++]; value = sl_tmp.ll0; } \
	else if (mode == STATE_SIZE) offset += 8; \
} while (0)
#define	SAVELOAD_BOOL(mode,offset,data,value) \
do { \
	if (mode == STATE_SAVE) data[offset++] = value? 0x01: 0x00; \
	else if (mode == STATE_LOAD) value = data[offset++]? true: false; \
	else if (mode == STATE_SIZE) offset++; \
} while (0)

