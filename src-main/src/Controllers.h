#pragma once

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#define	CONTROLLERS_MAXBUTTONS	32
#define	MAX_CONTROLLERS	32	// this includes keyboard and mouse

namespace Controllers {
extern	bool	capsLock;
extern	bool	scrollLock;

enum	STDCONT_TYPE
{
	STD_UNCONNECTED,
	STD_STDCONTROLLER,
	STD_ZAPPER,
	STD_ARKANOIDPADDLE,
	STD_POWERPADA,
	STD_POWERPADB,
	STD_FOURSCORE,
	STD_SNESCONTROLLER,
	STD_VSZAPPER,
	STD_SNESMOUSE,
	STD_SUBORMOUSE,
	STD_FOURSCORE2,
	STD_SUDOKU_EXCALIBUR,
	STD_SUDOKU_EXCALIBUR2,
	STD_PS2MOUSE,
	STD_YUXINGMOUSE,
	STD_BELSONICMOUSE,
	STD_MAX
};
class StdPort
{
public:
	virtual			~StdPort(void) {}
	virtual unsigned char	Read	(void) = 0;
	virtual void		Write	(unsigned char) = 0;
	virtual void		Config	(HWND) = 0;
	virtual void		Frame	(unsigned char) = 0;
	virtual void		SetMasks(void) = 0;
	virtual int		Save	(FILE *) = 0;
	virtual int		Load	(FILE *, int ver) = 0;
	virtual void            CPUCycle(void) = 0;

	STDCONT_TYPE	Type;
	DWORD		*Buttons;
	int		NumButtons;
	int		MovLen;
	unsigned char	*MovData;
};
#define DEF_STDCONT(NAME) \
struct StdPort_##NAME##_State; \
class StdPort_##NAME : public StdPort \
{ \
public: \
			StdPort_##NAME (DWORD *); \
			~StdPort_##NAME(void); \
	unsigned char	Read	(void); \
	void		Write	(unsigned char); \
	void		Config	(HWND); \
	void		Frame	(unsigned char); \
	void		SetMasks(void); \
	int		Save	(FILE *); \
	int		Load	(FILE *, int ver); \
	void            CPUCycle(void); \
	StdPort_##NAME##_State	*State; \
};
DEF_STDCONT(Unconnected)
DEF_STDCONT(StdController)
DEF_STDCONT(Zapper)
DEF_STDCONT(ArkanoidPaddle)
DEF_STDCONT(PowerPadA)
DEF_STDCONT(PowerPadB)
DEF_STDCONT(FourScore)
DEF_STDCONT(SnesController)
DEF_STDCONT(VSZapper)
DEF_STDCONT(SuborMouse)
DEF_STDCONT(SnesMouse)
DEF_STDCONT(FourScore2)
DEF_STDCONT(SudokuExcalibur)
DEF_STDCONT(SudokuExcalibur2)
DEF_STDCONT(PS2Mouse)
DEF_STDCONT(YuxingMouse)
DEF_STDCONT(BelsonicMouse)
#undef DEF_STDCONT

void	StdPort_SetControllerType (StdPort *&, STDCONT_TYPE, DWORD *);
#define SET_STDCONT(PORT,TYPE) StdPort_SetControllerType(PORT, TYPE, PORT##_Buttons)
extern const TCHAR	*StdPort_Mappings[STD_MAX];

enum	EXPCONT_TYPE {
	EXP_UNCONNECTED,
	EXP_FAMI4PLAY,
	EXP_ZAPPER,
	EXP_ARKANOIDPADDLE,
	EXP_FAMILYBASICKEYBOARD,
	EXP_SUBORKEYBOARD,
	EXP_FAMTRAINERA,
	EXP_FAMTRAINERB,
	EXP_TABLET,
	EXP_BANDAIHYPERSHOT,
	EXP_DONGDAKEYBOARD,
	EXP_TURBOFILE,
	EXP_BIT79KEYBOARD,
	EXP_CITYPATROLMAN,
	EXP_MOGURAA,
	EXP_KONAMIHYPERSHOT,
	EXP_SHARPC1CASSETTE,
	EXP_GOLDENNUGGETCASINO,
	EXP_KEDAKEYBOARD,
	EXP_HORI4PLAY,
	EXP_KINGWONKEYBOARD,
	EXP_ZECHENGKEYBOARD,
	EXP_JISSENMAHJONG,
	EXP_ABLPINBALL,
	EXP_TVPUMP,
	EXP_TRIFACEMAHJONG,
	EXP_MAHJONGGEKITOU,
	EXP_MAX
};
class ExpPort
{
public:
	virtual			~ExpPort(void) {}
	virtual unsigned char	Read1	(void) = 0;
	virtual unsigned char	Read2	(void) = 0;
	virtual unsigned char	ReadIOP	(uint8_t) = 0; // OneBus 410E-410F
	virtual void		Write	(unsigned char) = 0;
	virtual void		Config	(HWND) = 0;
	virtual void		Frame	(unsigned char) = 0;
	virtual void		SetMasks(void) = 0;
	virtual int		Save	(FILE *) = 0;
	virtual int		Load	(FILE *, int ver) = 0;
	virtual void		CPUCycle (void) = 0;
	EXPCONT_TYPE	Type;
	DWORD		*Buttons;
	int		NumButtons;
	int		MovLen;
	unsigned char	*MovData;
};
#define DEF_EXPCONT(NAME) \
struct ExpPort_##NAME##_State; \
class ExpPort_##NAME : public ExpPort \
{ \
public: \
			ExpPort_##NAME (DWORD *); \
			~ExpPort_##NAME(void); \
	unsigned char	Read1	(void); \
	unsigned char	Read2	(void); \
	unsigned char	ReadIOP	(uint8_t); \
	void		Write	(unsigned char); \
	void		Config	(HWND); \
	void		Frame	(unsigned char); \
	void		SetMasks(void); \
	int		Save	(FILE *); \
	int		Load	(FILE *, int ver); \
	void		CPUCycle (void); \
	ExpPort_##NAME##_State	*State; \
};
DEF_EXPCONT(Unconnected)
DEF_EXPCONT(Fami4Play)
DEF_EXPCONT(ArkanoidPaddle)
DEF_EXPCONT(FamilyBasicKeyboard)
DEF_EXPCONT(SuborKeyboard)
DEF_EXPCONT(FamTrainerA)
DEF_EXPCONT(FamTrainerB)
DEF_EXPCONT(Tablet)
DEF_EXPCONT(Zapper)
DEF_EXPCONT(BandaiHyperShot)
DEF_EXPCONT(DongdaKeyboard)
DEF_EXPCONT(TurboFile)
DEF_EXPCONT(Bit79Keyboard)
DEF_EXPCONT(CityPatrolman)
DEF_EXPCONT(Moguraa)
DEF_EXPCONT(KonamiHyperShot)
DEF_EXPCONT(SharpC1Cassette)
DEF_EXPCONT(GoldenNuggetCasino)
DEF_EXPCONT(KedaKeyboard)
DEF_EXPCONT(Hori4Play)
DEF_EXPCONT(KingwonKeyboard)
DEF_EXPCONT(ZeChengKeyboard)
DEF_EXPCONT(JissenMahjong)
DEF_EXPCONT(ABLPinball)
DEF_EXPCONT(TVPump)
DEF_EXPCONT(TrifaceMahjong)
DEF_EXPCONT(MahjongGekitou)
#undef DEF_EXPCONT

void	ExpPort_SetControllerType (ExpPort *&, EXPCONT_TYPE, DWORD *);
#define SET_EXPCONT(PORT,TYPE) ExpPort_SetControllerType(PORT, TYPE, PORT##_Buttons)
extern const TCHAR	*ExpPort_Mappings[EXP_MAX];

enum	JOY_AXIS	{ AXIS_X, AXIS_Y, AXIS_Z, AXIS_RX, AXIS_RY, AXIS_RZ, AXIS_S0, AXIS_S1 };

extern StdPort *Port1, *Port2;
extern StdPort *FSPort1, *FSPort2, *FSPort3, *FSPort4;
extern ExpPort *PortExp;

extern DWORD	Port1_Buttons[CONTROLLERS_MAXBUTTONS],
		Port2_Buttons[CONTROLLERS_MAXBUTTONS];
extern DWORD	FSPort1_Buttons[CONTROLLERS_MAXBUTTONS],
		FSPort2_Buttons[CONTROLLERS_MAXBUTTONS],
		FSPort3_Buttons[CONTROLLERS_MAXBUTTONS],
		FSPort4_Buttons[CONTROLLERS_MAXBUTTONS];
extern DWORD	PortExp_Buttons[CONTROLLERS_MAXBUTTONS];

extern BOOL	EnableOpposites;

extern BYTE		KeyState[256];
extern DIMOUSESTATE2	MouseState;

void	OpenConfig (void);
void	Init (void);
void	Destroy (void);
void	LoadButtons (HKEY);
void	SaveButtons (HKEY);
void	SaveSettings (HKEY);
void	LoadSettings (HKEY);
void	Write (unsigned char);
int	Save (FILE *);
int	Load (FILE *, int ver);
void	SetDeviceUsed (void);
void	Acquire (void);
void	UnAcquire (void);
void	UpdateInput (void);
void	ConfigButton (DWORD *, int, HWND, BOOL, BOOL = FALSE);

BOOL	IsPressed (int);
int	GetDelta (int);
INT_PTR	ParseConfigMessages (HWND, UINT, WPARAM, LPARAM, int, int, const int *, const int *, DWORD *, bool);
//void	SetControllersAutomatically (void);
void	SwitchControllersAutomatically (void);
void	SetDefaultInput(uint8_t value);
} // namespace Controllers
