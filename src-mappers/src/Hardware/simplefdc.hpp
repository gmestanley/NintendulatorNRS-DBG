#include <queue>
struct Drive {
	bool	exists;
	bool	ready;
	bool	inserted;
	bool	running;
	bool	writeProtected;
	bool	changed;
	int	cylinder;
	int	cylinders;
	int	sides;
	int	sectors;
	int	correctDataRate;
	uint8_t *data;
	bool	*modifiedFlag;
};

#pragma warning(disable:4121)	// "alignment of a member was sensitive to packing" - most useless warning ever

class FDC;
struct CommandInfo {
	uint8_t length;
	void (FDC::*handler)(void);
};

class FDC {
	Drive drives[4], *drive;
	uint8_t DOR, MSR, ST0, ST1, ST2, SRA, CONFIG, latch;
	uint8_t cylinder, head, sectorNumber, *data;
	bool	resetPin, useDMA, dmaRequest, dmaTerminate;
	int	dataRate, sectorSize, bytesLeft;
	void	(FDC::*phaseHandler)(void);
	std::vector<uint8_t> command;
	std::queue<uint8_t> resultQueue;
	static const CommandInfo commandInfo[0x20];
	
	// Polling
	int	pollTimer;
	uint8_t	pollDrive;
	bool	previouslyReady[4];

	void	phCommand (void);
	void	phResult (void);
	void	endCommand (bool, int, ...);
	void	nextSector (void);
	void	nextSectorFormat (void);
	void	cmReadID (void);
	void	cmReadData (void);
	void	cmWriteData (void);
	void	cmFormatTrack (void);
	void	cmSeek (void);
	void	cmRecalibrate (void);
	void	cmSenseInterruptStatus (void);
	void	cmSpecify (void);
	void	cmSenseDriveStatus (void);
	void	cmConfigure (void);
	void	cmInvalid (void);
	void	pollDrives (void);
public:
	FDC();
	void	insertDisk (uint8_t, uint8_t*, size_t, bool, bool*);
	void	ejectDisk (uint8_t);
	void	reset();
	void	run (void);
	int	readIO (int);
	int	readIODebug (int);
	void	writeIO (int, int);
	bool	irqRaised (void) const;
	bool	haveDMARequest (void) const;
	void	setTerminalCount (void);
	void	sendToDMA (uint8_t);
	uint8_t	receiveFromDMA (void);
	void	setDriveMask (uint8_t);
	void	setResetPin (bool);
};
