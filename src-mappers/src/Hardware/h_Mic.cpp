#include "h_Mic.h"
#include <Mmsystem.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <mmeapi.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != NULL)        \
   {                    \
      x->Release();     \
      x = NULL;         \
   }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
   if(x != NULL)             \
   {                         \
      delete[] x;            \
      x = NULL;              \
   }
#endif


namespace Mic {
BOOL			initialized = 0;
IMMDeviceEnumerator*	pEnumerator = NULL;
IMMDevice*		pDevice = NULL;
IAudioMeterInformation*	pMeterInfo = NULL;
HWAVEIN			hwi;

BOOL	MAPINT	Load (void) {
	if (!initialized) {
		WAVEFORMATEX wfx;
		wfx.nChannels =1;
		wfx.nSamplesPerSec =44100;
		wfx.wFormatTag =WAVE_FORMAT_PCM;
		wfx.wBitsPerSample  = 8;                                        
		wfx.nBlockAlign     = wfx.nChannels * wfx.wBitsPerSample / 8;   
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;     
		wfx.cbSize          = 0;
		MMRESULT mmresult =waveInOpen(&hwi, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL);
		if (mmresult !=MMSYSERR_NOERROR) {
			switch(mmresult) {
			case MMSYSERR_ALLOCATED: EMU->DbgOut(L"Mic: Specified resource is already allocated."); break;
			case MMSYSERR_BADDEVICEID: EMU->DbgOut(L"Mic: Specified device identifier is out of range."); break;
			case MMSYSERR_NODRIVER: EMU->DbgOut(L"Mic: No device driver is present."); break;
			case MMSYSERR_NOMEM: EMU->DbgOut(L"Mic: Unable to allocate or lock memory."); break;
			case WAVERR_BADFORMAT: EMU->DbgOut(L"Mic: Attempted to open with an unsupported waveform-audio format. ."); break;
			}
			return FALSE;
		}			
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator) !=S_OK) {
			EMU->DbgOut(L"Mic: Cannot create instance of MMDeviceEnumerator.");
			CoUninitialize();
			waveInClose(hwi);
			return FALSE;
		}
		pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
		pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
		initialized =1;
	}
	return TRUE;
}

float	MAPINT	Read (void) {
	float peak = 0.0;
	if (initialized) pMeterInfo->GetPeakValue(&peak);
	return peak;
}

void	MAPINT	Unload (void) {
	if (initialized) {
		SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(pMeterInfo)
		CoUninitialize();
		waveInClose(hwi);
		initialized =0;
	}
}

} // namespace