#include <DSound.h>

#include "CDirectSound.h"


#ifndef DSBLOCK_ENTIREBUFFER
#define DSBLOCK_ENTIREBUFFER        0x00000002
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define new DEBUG_NEW
#endif





LPDIRECTSOUND CDirectSound::m_lpDirectSound;
DWORD CDirectSound::m_dwInstances;

void checkPl(int length_, HWND hwnd) {
	Sleep(length_);
	PostMessage(hwnd, WM_PLAYOVER, 0, 0);
}


CDirectSound::CDirectSound()
{
	m_lpDirectSound = 0;
	m_pDsb = 0;
	m_pTheSound = 0;
	m_dwTheSound = 0;
	m_bEnabled = TRUE;

	++m_dwInstances;
}

CDirectSound::~CDirectSound()
{
	if (m_pDsb)
		m_pDsb->Release();

	if (!--m_dwInstances && m_lpDirectSound) {
		m_lpDirectSound->Release();
		m_lpDirectSound = 0;
	}
	std::string cmd;
	cmd = "close " + alias;
	mci.send(cmd);
}

BOOL CDirectSound::Create(LPCTSTR pszResource, HWND pWnd)
{
	Mp3 = false;
	//////////////////////////////////////////////////////////////////
	// load resource
	HINSTANCE hApp = ::GetModuleHandle(0);
	assert(hApp);

	HRSRC hResInfo = ::FindResource(hApp, pszResource, TEXT("WAVE"));
	if (hResInfo == 0)
		return FALSE;

	HGLOBAL hRes = ::LoadResource(hApp, hResInfo);
	if (hRes == 0)
		return FALSE;

	LPVOID pTheSound = ::LockResource(hRes);
	if (pTheSound == 0)
		return FALSE;

	return Create(pTheSound, pWnd);
}


BOOL CDirectSound::Create(LPVOID pSoundData, HWND pWnd) {
	/*if (pWnd == 0)
		pWnd = pWnd;

	assert(pWnd != 0);
	assert(::IsWindow(pWnd));*/

	assert(pSoundData != 0);

	//////////////////////////////////////////////////////////////////
	// create direct sound object

	if (m_lpDirectSound == 0) {
		// Someone might use sounds for starting apps. This may cause
		// DirectSoundCreate() to fail because the driver is used by
		// anyone else. So wait a little before starting with the work ...
		HRESULT hRes = DS_OK;
		short nRes = 0;

		do {
			if (nRes)
				::Sleep(500);
			hRes = ::DirectSoundCreate(0, &m_lpDirectSound, 0);
			++nRes;
		} while (nRes < 10 && (hRes == DSERR_ALLOCATED || hRes == DSERR_NODRIVER));

		if (hRes != DS_OK)
			return FALSE;

		m_lpDirectSound->SetCooperativeLevel(pWnd, DSSCL_NORMAL);
	}

	assert(m_lpDirectSound != 0);
	
	WAVEFORMATEX * pcmwf;
	if (!GetWaveData(pSoundData, pcmwf, m_pTheSound, m_dwTheSound) ||
		!CreateSoundBuffer(pcmwf) ||
		!SetSoundData(m_pTheSound, m_dwTheSound)) {
		
		return FALSE;
	}

	return TRUE;
}


BOOL CDirectSound::GetWaveData(void * pRes, WAVEFORMATEX * & pWaveHeader, void * & pbWaveData, DWORD & cbWaveSize) {
	pWaveHeader = 0;
	pbWaveData = 0;
	cbWaveSize = 0;

	DWORD * pdw = (DWORD *)pRes;
	DWORD dwRiff = *pdw++;
	DWORD dwLength = *pdw++;
	DWORD dwType = *pdw++;

	if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
		return FALSE;      // not even RIFF

	if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
		return FALSE;      // not a WAV

	DWORD * pdwEnd = (DWORD *)((BYTE *)pdw + dwLength - 4);

	while (pdw < pdwEnd) {
		dwType = *pdw++;
		dwLength = *pdw++;

		switch (dwType) {
		case mmioFOURCC('f', 'm', 't', ' '):
			if (!pWaveHeader) {
				if (dwLength < sizeof(WAVEFORMAT))
					return FALSE;      // not a WAV

				pWaveHeader = (WAVEFORMATEX *)pdw;

				if (pbWaveData && cbWaveSize)
					return TRUE;
			}
			break;

		case mmioFOURCC('d', 'a', 't', 'a'):
			pbWaveData = LPVOID(pdw);
			cbWaveSize = dwLength;

			if (pWaveHeader)
				return TRUE;
			break;
		}
		pdw = (DWORD *)((BYTE *)pdw + ((dwLength + 1)&~1));
	}

	return FALSE;
}


BOOL CDirectSound::CreateSoundBuffer(WAVEFORMATEX * pcmwf)
{
	DSBUFFERDESC dsbdesc;

	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	// Need no controls (pan, volume, frequency).
	dsbdesc.dwFlags = DSBCAPS_STATIC;  // assumes that the sound is played often
	dsbdesc.dwBufferBytes = m_dwTheSound;
	dsbdesc.lpwfxFormat = pcmwf;    // Create buffer.
	HRESULT hRes;
	if (DS_OK != (hRes = m_lpDirectSound->CreateSoundBuffer(&dsbdesc, &m_pDsb, 0))) {
		// Failed.
		MessageBox(NULL, "Fail", "Fail", MB_OK);
		m_pDsb = 0;
		return FALSE;
	}

	return TRUE;
}


BOOL CDirectSound::SetSoundData(LPVOID  pSoundData, DWORD dwSoundSize) {
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	// Obtain write pointer.
	HRESULT hr = m_pDsb->Lock(0, 0, &lpvPtr1, &dwBytes1, 0, 0, DSBLOCK_ENTIREBUFFER);
	// If DSERR_BUFFERLOST is returned, restore and retry lock.
	if (DSERR_BUFFERLOST == hr) {
		m_pDsb->Restore();
		hr = m_pDsb->Lock(0, 0, &lpvPtr1, &dwBytes1, 0, 0, DSBLOCK_ENTIREBUFFER);
	}
	if (DS_OK == hr) {
		// Write to pointers.
		::CopyMemory(lpvPtr1, pSoundData, dwBytes1);
		// Release the data back to DirectSound.
		hr = m_pDsb->Unlock(lpvPtr1, dwBytes1, 0, 0);
		if (DS_OK == hr)
			return TRUE;
	}
	// Lock, Unlock, or Restore failed.
	return FALSE;
}

void CDirectSound::Play(DWORD dwStartPosition, BOOL bLoop)
{
	if (!IsValid() || !IsEnabled()) {
		return;  // no chance to play the sound ...
	}

	if (dwStartPosition > m_dwTheSound)
		dwStartPosition = m_dwTheSound;
	m_pDsb->SetCurrentPosition(dwStartPosition);
	if (DSERR_BUFFERLOST == m_pDsb->Play(0, 0, bLoop ? DSBPLAY_LOOPING : 0)) {
		// another application had stolen our buffer
		// Note that a "Restore()" is not enough, because
		// the sound data is invalid after Restore().
		SetSoundData(m_pTheSound, m_dwTheSound);

		// Try playing again
		m_pDsb->Play(0, 0, bLoop ? DSBPLAY_LOOPING : 0);
	}
}

void CDirectSound::Stop()
{	
	if (Mp3 == false) {
		if (IsValid())
				m_pDsb->Stop();
	}
	else {
		std::string cmd;
		cmd = "stop " + alias;
		mci.send(cmd);
		cmd = "seek " + alias + " to start";
		mci.send(cmd);
	}
}

void CDirectSound::Pause()
{
	if(Mp3==false)
		Stop();
	else {
		std::string cmd;
		cmd = "pause " + alias;
	}
}

void CDirectSound::Continue()
{
	if (Mp3 == false) {
		if (IsValid()) {
			DWORD dwPlayCursor, dwWriteCursor;
			m_pDsb->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);
			Play(dwPlayCursor);
		}
	}
	else {
		std::string cmd;
		cmd = "resume " + alias;
		mci.send(cmd);

	}
}

BOOL CDirectSound::IsValid() const
{
	return (m_lpDirectSound && m_pDsb && m_pTheSound && m_dwTheSound) ? TRUE : FALSE;
}



BOOL CDirectSound::Create(const char* filename_,bool MP3_,HWND hwnd) {
	hwnd_ = hwnd;
	Mp3 = MP3_;
	filename = filename_;
	for (unsigned int i = 0; i < filename.length(); i++)
	{
		if (filename[i] == '/')
			filename[i] = '\\';
	}
	alias = "mp3_";
	srand(time(NULL));
	char randstr[6];
	_itoa(rand() % 65536, randstr, 10);
	alias.append(randstr);
	
	std::string cmd;
	cmd = "open " + filename + " alias " + alias;

	if (mci.send(cmd) == false)
		return false;

	cmd = "set " + alias + " time format milliseconds";
	if (mci.send(cmd) == false)
		return false;

	cmd = "status " + alias + " length";
	if (mci.send(cmd) == false)
		return false;

	length_ms = atoi(mci.buf);

	return true;
}

void CDirectSound::Play(int start_ms , int end_ms,bool LOOP ) {
	if (end_ms == -1) end_ms = length_ms;
	std::string cmd;
	char start_str[16], end_str[16];
	_itoa(start_ms, start_str, 10);
	_itoa(end_ms, end_str, 10);
	cmd = "play " + alias + " from ";
	cmd.append(start_str);
	cmd.append(" to ");
	cmd.append(end_str);
	mci.send(cmd);

	if (LOOP == 1) {
		std::thread task1(checkPl, length_ms, hwnd_);
		task1.detach();
	}

}

Mci::Mci()
{
	HINSTANCE hins = LoadLibraryA("winmm.dll");
	wmci = (w32mci)GetProcAddress(hins, "mciSendStringA");
	wmcierror = (w32mcierror)GetProcAddress(hins, "mciGetErrorStringA");
}
Mci::~Mci()
{
	FreeLibrary(hins);
}
bool Mci::send(std::string command)
{
	int errcode = wmci(command.c_str(), buf, 254, 0);
	if (errcode)
	{
		wmcierror(errcode, buf, 254);
		return false;
	}
	return true;
}