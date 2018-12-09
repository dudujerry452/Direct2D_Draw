#pragma once

//header:
#include <Dsound.h>
#include <wincodec.h>

#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
//#include "mpg123.h"
#include <mmsystem.h>
#include <Dsound.h>
#include <tchar.h>
#include <assert.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>


//#include <d2d1effects_2.h>

#include <cstdlib>
#include <cstdio>

#pragma comment(lib,"D2D1.lib")
#pragma comment(lib,"WindowsCodecs.lib")
#pragma comment(lib,"DWrite.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"Dsound.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"odbc32.lib")
#pragma comment(lib,"odbccp32.lib")
#pragma comment(lib,"dxguid.lib")
//#pragma comment(lib,"dxerr.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

#define DColor(A) D2D1::ColorF( D2D1::ColorF::A )



typedef ID2D1Bitmap D2DBitMap;
typedef IDWriteTextFormat TextFormat;

typedef D2D1_SIZE_U BMPRect;


struct RECT_F {
	float left;
	float top;
	float right;
	float bottom;

	RECT_F(float L, float T, float R, float B)
	{
		left = L;
		top = T;
		right = R;
		bottom = B;
	}
};

struct RGBA_F {
	float R;
	float G;
	float B;
	float A;

	RGBA_F(float r, float g, float b, float a)
	{
		R = r;
		G = g;
		B = b;
		A = a;

	}

};

template<typename T>
inline void SafeRelease(T &ToRel)
{
	if (ToRel != NULL)
	{
		ToRel->Release();

		ToRel = NULL;
	}
}


class D2DDraw
{
public:
	D2DDraw();
	~D2DDraw();

	void LoadD2D(HWND hwnd);
	void BeginDr();
	void EndDr();

	D2DBitMap* LoadBMP(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight);
	void DrawBMP(D2DBitMap* BMP, int X, int Y, float Alpha);
	void DrawBMP(D2DBitMap* BMP, int X, int Y, int Width, int Height, float Alpha,bool enlarge);
	
	void ClearScr(const D2D1_COLOR_F COL);
	D2DBitMap* LoadAlphaBMP(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight, RGBA_F TransColor);

	D2DBitMap* LoadPNG(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight);
	D2DBitMap* LoadPNGA(LPCWSTR FileName, UINT destinationWidth, UINT destinationHeight);

	TextFormat* LoadText(LPCWSTR Typeface, float Size, LPCWSTR Language);
	void D2DDrawText(D2D1::ColorF Color, RECT_F TextRC, LPCWSTR Text, TextFormat* Format);

	//void SetTrans(float Trans);

	void DrawRect(RECT_F reCt, D2D1::ColorF Color);

	BMPRect GetBMPRect(D2DBitMap* bmp);

private:
	//bool TFEffect;

	ID2D1Factory * pD2DFac;
	ID2D1HwndRenderTarget* pRT;
	ID2D1SolidColorBrush* pBrush;

	IWICBitmap*    m_pWicBitmap;
	IWICImagingFactory* m_pWicImagingFactory;
	IWICBitmapDecoder* m_pWicDecoder;
	IWICBitmapFrameDecode* m_pWicFrameDecoder;
	IWICBitmapScaler* p_Scaler;

	//ID2D1Layer* g_pLayer;

	RECT rc;

	ID2D1SolidColorBrush* pIWriBrush;
	IDWriteFactory* pIDWriteFactory;
	ID2D1SolidColorBrush* pWriBrush;

	ID2D1SolidColorBrush* rBrush;

	void ReleaseBMP();
	void ReleaseText();
};


