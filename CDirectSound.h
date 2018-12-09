#pragma once
#if !defined(AFX_DIRECTSOUND_H__A20FE86F_118F_11D2_9AB3_0060B0CDC13E__INCLUDED_)
#define AFX_DIRECTSOUND_H__A20FE86F_118F_11D2_9AB3_0060B0CDC13E__INCLUDED_

#define WM_PLAYOVER WM_USER+1

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <mmsystem.h>
#include <dsound.h>
#include <assert.h>
#include <windows.h>
//#include "mpg123.h"
#include <string>
#include <ctime>
#include <thread>
#include <mutex>
#include <cstring>

#pragma message("linking with Microsoft's DirectSound library ...")
#pragma comment(lib, "dsound.lib")
//#pragma comment(lib,"./libmpg123.lib")
#pragma warning(disable:4996)

typedef  int(__stdcall* w32mci)(const char*, char*, int, int);
typedef int(__stdcall *  w32mcierror)(int, char*, int);



class Mci
{
private:
	HINSTANCE hins;
	w32mci wmci;
	w32mcierror wmcierror;
public:
	Mci();
	~Mci();
	char buf[256];
	bool send(std::string command);//error  return false 
};

class CDirectSound
{
public:  // construction/destruction
	CDirectSound();
	virtual ~CDirectSound();

	

	//If the "pWnd" paramter is NULL, then AfxGetApp()->GetMainWnd() will be used.
	BOOL Create(LPCTSTR pszResource, HWND pWnd = 0);
	BOOL Create(UINT uResourceID, HWND pWnd = 0) {
		Mp3 = false;
		//std::string str = std::to_string(uResourceID);
		//MessageBox(NULL, str.c_str(), "debug", MB_OK);
		return Create(MAKEINTRESOURCE(uResourceID), pWnd);
	}
	// Alternativly you can specify the sound by yourself
	// Note that the class does not copy the entire data ! Instead
	// a pointer to the given data will be stored !
	// You can load an entire WAV file into memory and then call this
	// Create() method.
	BOOL Create(LPVOID pSoundData, HWND pWnd = 0);

	BOOL Create(const char* filename_,bool MP3_,HWND hwnd);
	


public:  // operations
	HWND hwnd_;
	std::string alias;
	std::string filename;

	Mci mci;

	bool Mp3;
	int length_ms;

	BOOL   IsValid() const;
	void   Play(DWORD dwStartPosition = 0, BOOL bLoop = FALSE);
	void   Play(int start_ms , int end_ms ,bool LOOP);
	void   Stop();
	void   Pause();
	void   Continue();
	CDirectSound & EnableSound(BOOL bEnable = TRUE) {
		m_bEnabled = bEnable;
		if (!bEnable)
			Stop();
		return *this;
	}
	BOOL   IsEnabled() const { return m_bEnabled; }

protected: // implementation
	BOOL SetSoundData(LPVOID pSoundData, DWORD dwSoundSize);
	BOOL CreateSoundBuffer(WAVEFORMATEX * pcmwf);
	BOOL GetWaveData(void * pRes, WAVEFORMATEX * & pWaveHeader, void * & pbWaveData, DWORD & cbWaveSize);

private: // data member
	LPVOID m_pTheSound;
	DWORD m_dwTheSound;
	LPDIRECTSOUNDBUFFER m_pDsb;
	BOOL m_bEnabled;
	static LPDIRECTSOUND m_lpDirectSound;
	static DWORD m_dwInstances;
};

#endif // !defined(AFX_DIRECTSOUND_H__A20FE86F_118F_11D2_9AB3_0060B0CDC13E__INCLUDED_)